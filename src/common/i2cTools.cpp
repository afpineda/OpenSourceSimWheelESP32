/**
 * @file i2c.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-08-16
 * @brief I2C bus initialization and common utilities
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "i2cTools.h"
#include "driver/i2c.h"

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

#define STANDARD_CLOCK_SPEED 100000
static gpio_num_t sdaPin[] = {(gpio_num_t)SDA, GPIO_NUM_NC};
static gpio_num_t sclPin[] = {(gpio_num_t)SCL, GPIO_NUM_NC};
static bool isInitialized[] = {false, false};
static uint8_t max_speed_x[] = {4, 4};

// ----------------------------------------------------------------------------
// Auxiliary
// ----------------------------------------------------------------------------

bool doInitializeI2C(gpio_num_t sda, gpio_num_t scl, uint8_t clock_multiplier, i2c_port_t bus)
{
    i2c_config_t conf;
    memset(&conf, 0, sizeof(i2c_config_t));
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sda;
    conf.scl_io_num = scl;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = STANDARD_CLOCK_SPEED * clock_multiplier;
    if (i2c_param_config(bus, &conf) == ESP_OK)
        if (i2c_driver_install(bus, I2C_MODE_MASTER, 0, 0, 0) == ESP_OK)
            return true;
    return false;
}

// ----------------------------------------------------------------------------

void checkSpeedMultiplier(uint8_t &max_speed_multiplier)
{
    if (max_speed_multiplier < 1)
        max_speed_multiplier = 1;
    if (max_speed_multiplier > 4)
        max_speed_multiplier = 4;
}

// ----------------------------------------------------------------------------

void i2cError(gpio_num_t sda, gpio_num_t scl, uint8_t clock_multiplier, i2c_port_t bus)
{
    log_e("Unable to initialize I2C bus #%d, SDA=%d, SCL=%d, clock multiplier=%d",
          bus, sda, scl, clock_multiplier);
    abort();
}

// ----------------------------------------------------------------------------
// I2C probe
// ----------------------------------------------------------------------------

bool i2c::probe(uint8_t address7bits, bool secondaryBus)
{
    i2c::abortOnInvalidAddress(address7bits);
    i2c_port_t bus = secondaryBus ? I2C_NUM_1 : I2C_NUM_0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address7bits << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    bool result = (i2c_master_cmd_begin(bus, cmd, 500 / portTICK_RATE_MS) == ESP_OK);
    i2c_cmd_link_delete(cmd);
    return result;
}

// ----------------------------------------------------------------------------

void i2c::probe(std::vector<uint8_t> &result, bool secondaryBus)
{
    result.clear();
    i2c_port_t bus = secondaryBus ? I2C_NUM_1 : I2C_NUM_0;
    if (isInitialized[bus])
        // Deinitialize
        ESP_ERROR_CHECK(i2c_driver_delete(bus));

    // Initialize to minimum speed
    if (!doInitializeI2C(sdaPin[bus], sclPin[bus], 1, bus))
        i2cError(sdaPin[bus], sclPin[bus], 1, bus);

    // Probe
    for (uint8_t address = 0; address < 128; address++)
    {
        if (i2c::probe(address, secondaryBus))
            result.push_back(address);
    }

    // Deinitialize
    ESP_ERROR_CHECK(i2c_driver_delete(bus));

    if (isInitialized[bus])
        // Reinitialize
        if (!doInitializeI2C(sdaPin[bus], sclPin[bus], max_speed_x[bus], bus))
            i2cError(sdaPin[bus], sclPin[bus], max_speed_x[bus], bus);
}

// ----------------------------------------------------------------------------
// Device initialization
// ----------------------------------------------------------------------------

void i2c::require(uint8_t max_speed_multiplier, bool secondaryBus)
{
    checkSpeedMultiplier(max_speed_multiplier);
    i2c_port_t bus = secondaryBus ? I2C_NUM_1 : I2C_NUM_0;
    if (isInitialized[bus])
    {
        // check clock compatibility
        if (max_speed_multiplier < max_speed_x[bus])
        {
            // Deinitialize
            ESP_ERROR_CHECK(i2c_driver_delete(bus));
        }
        else
            // Already initalized
            return;
    }
    // Initialize
    if (doInitializeI2C(sdaPin[bus], sclPin[bus], max_speed_multiplier, bus))
    {
        isInitialized[bus] = true;
        max_speed_x[bus] = max_speed_multiplier;
    }
    else
        i2cError(sdaPin[bus], sclPin[bus], max_speed_multiplier, bus);
}

// ----------------------------------------------------------------------------
// User initialization
// ----------------------------------------------------------------------------

void i2c::begin(gpio_num_t sda, gpio_num_t scl, bool secondaryBus)
{
    i2c_port_t bus = secondaryBus ? I2C_NUM_1 : I2C_NUM_0;
    sdaPin[bus] = sda;
    sclPin[bus] = scl;
    if (isInitialized[bus])
    {
        // Deinitialize
        ESP_ERROR_CHECK(i2c_driver_delete(bus));
        // Initialize again with new pins
        isInitialized[bus] = doInitializeI2C(sdaPin[bus], sclPin[bus], max_speed_x[bus], bus);
        if (!isInitialized[bus])
            i2cError(sdaPin[bus], sclPin[bus], max_speed_x[bus], bus);
    }
}

// ----------------------------------------------------------------------------
// Checks
// ----------------------------------------------------------------------------

void i2c::abortOnInvalidAddress(
    uint8_t address7bits,
    uint8_t minAddress,
    uint8_t maxAddress)
{
    if (minAddress > maxAddress)
    {
        i2c::abortOnInvalidAddress(address7bits, maxAddress, minAddress);
        return;
    }
    if ((address7bits < minAddress) || (address7bits > maxAddress))
    {
        log_e("Not a valid I2C address: %x (hex)", address7bits);
        abort();
    }
}

// ----------------------------------------------------------------------------
// Hardware addresses
// ----------------------------------------------------------------------------

uint8_t i2c::findFullAddress(
    std::vector<uint8_t> &fullAddressList,
    uint8_t hardwareAddress,
    uint8_t hardwareAddressMask)
{
    uint8_t fullAddress = 0xFF;
    int count = 0;

    // Find full addresses matching the given hardware address
    for (int idx = 0; idx < fullAddressList.size(); idx++)
    {
        uint8_t candidate = fullAddressList.at(idx) & hardwareAddressMask;
        if (candidate == hardwareAddress)
        {
            count++;
            fullAddress = fullAddressList.at(idx);
        }
    }

    if (count == 0)
        return 0xFF;
    else if (count > 1)
        return 0xFE;
    else
        return fullAddress;
}