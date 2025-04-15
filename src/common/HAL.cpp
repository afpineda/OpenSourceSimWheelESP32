/**
 * @file HAL.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-05
 * @brief Abstraction of the underlying ESP-IDF API
 *
 * @copyright Licensed under the EUPL
 *
 */

#if CD_CI
#error Arduino only
#endif

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "HAL.hpp"
#include "driver/i2c.h"          // For I2C operation
#include "esp32-hal-log.h"       // For log_e()
#include "esp_adc/adc_oneshot.h" // For ADC operation
#include "esp32-hal.h"           // For SDA and SCL pin definitions
#include "driver/gpio.h"         // For gpio_set_level/gpio_get_level()
#include "esp_intr_alloc.h"      // For gpio_isr_handler_add()
#include "esp32-hal-cpu.h"       // For getCpuFrequencyMhz()

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

// I2C
#define STANDARD_CLOCK_SPEED 100000
static gpio_num_t sdaPin[] = {(gpio_num_t)SDA, GPIO_NUM_NC};
static gpio_num_t sclPin[] = {(gpio_num_t)SCL, GPIO_NUM_NC};
static bool isInitialized[] = {false, false};
static uint8_t max_speed_x[] = {4, 4};

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// I2C
//-------------------------------------------------------------------
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// I2C: Auxiliary
//-------------------------------------------------------------------

bool doInitializeI2C(
    gpio_num_t sda,
    gpio_num_t scl,
    uint8_t clock_multiplier,
    i2c_port_t _bus)
{
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sda;
    conf.scl_io_num = scl;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = STANDARD_CLOCK_SPEED * clock_multiplier;
    if (i2c_param_config(_bus, &conf) == ESP_OK)
        if (i2c_driver_install(_bus, I2C_MODE_MASTER, 0, 0, 0) == ESP_OK)
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

void i2cError(
    gpio_num_t sda,
    gpio_num_t scl,
    uint8_t clock_multiplier,
    i2c_port_t _bus)
{
    throw i2c_error(sda, scl, (int)_bus, clock_multiplier);
}

//-------------------------------------------------------------------
// I2C: Probe
//-------------------------------------------------------------------

bool internals::hal::i2c::probe(uint8_t address7bits, I2CBus bus)
{
    internals::hal::i2c::abortOnInvalidAddress(address7bits);
    auto _bus = static_cast<i2c_port_t>(bus);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address7bits << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    bool result = (i2c_master_cmd_begin(_bus, cmd, 500 / portTICK_RATE_MS) == ESP_OK);
    i2c_cmd_link_delete(cmd);
    return result;
}

// ----------------------------------------------------------------------------

void internals::hal::i2c::probe(std::vector<uint8_t> &result, I2CBus bus)
{
    auto _bus = static_cast<i2c_port_t>(bus);
    result.clear();
    if (isInitialized[_bus])
        // Deinitialize
        ESP_ERROR_CHECK(i2c_driver_delete(_bus));

    // Initialize to minimum speed
    if (!doInitializeI2C(sdaPin[_bus], sclPin[_bus], 1, _bus))
        i2cError(sdaPin[_bus], sclPin[_bus], 1, _bus);

    // Probe
    for (uint8_t address = 0; address < 128; address++)
    {
        if (internals::hal::i2c::probe(address, bus))
            result.push_back(address);
    }

    // Deinitialize
    ESP_ERROR_CHECK(i2c_driver_delete(_bus));

    if (isInitialized[_bus])
        // Reinitialize
        if (!doInitializeI2C(sdaPin[_bus], sclPin[_bus], max_speed_x[_bus], _bus))
            i2cError(sdaPin[_bus], sclPin[_bus], max_speed_x[_bus], _bus);
}

// ----------------------------------------------------------------------------
// I2C: bus initialization
// ----------------------------------------------------------------------------

void internals::hal::i2c::initialize(GPIO sda, GPIO scl, I2CBus bus)
{
    auto _bus = static_cast<i2c_port_t>(bus);
    sdaPin[_bus] = static_cast<gpio_num_t>((int)sda);
    sclPin[_bus] = static_cast<gpio_num_t>((int)scl);
    if (isInitialized[_bus])
    {
        // Deinitialize
        ESP_ERROR_CHECK(i2c_driver_delete(_bus));
        // Initialize again with new pins
        isInitialized[_bus] = doInitializeI2C(sdaPin[_bus], sclPin[_bus], max_speed_x[_bus], _bus);
        if (!isInitialized[_bus])
            i2cError(sdaPin[_bus], sclPin[_bus], max_speed_x[_bus], _bus);
    }
}

void internals::hal::i2c::require(uint8_t max_speed_multiplier, I2CBus bus)
{
    auto _bus = static_cast<i2c_port_t>(bus);
    checkSpeedMultiplier(max_speed_multiplier);
    if (isInitialized[_bus])
    {
        // check clock compatibility
        if (max_speed_multiplier < max_speed_x[_bus])
        {
            // Deinitialize
            ESP_ERROR_CHECK(i2c_driver_delete(_bus));
        }
        else
            // Already initalized
            return;
    }
    // Initialize
    if (doInitializeI2C(sdaPin[_bus], sclPin[_bus], max_speed_multiplier, _bus))
    {
        isInitialized[_bus] = true;
        max_speed_x[_bus] = max_speed_multiplier;
    }
    else
        i2cError(sdaPin[_bus], sclPin[_bus], max_speed_multiplier, _bus);
}

// ----------------------------------------------------------------------------
// I2C Checks
// ----------------------------------------------------------------------------

void internals::hal::i2c::abortOnInvalidAddress(
    uint8_t address7bits,
    uint8_t minAddress,
    uint8_t maxAddress)
{
    if (minAddress > maxAddress)
    {
        internals::hal::i2c::abortOnInvalidAddress(address7bits, maxAddress, minAddress);
        return;
    }
    if ((address7bits < minAddress) || (address7bits > maxAddress))
    {
        throw i2c_error(address7bits);
    }
}

// ----------------------------------------------------------------------------
// Hardware addresses
// ----------------------------------------------------------------------------

uint8_t internals::hal::i2c::findFullAddress(
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

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// GPIO
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

int internals::hal::gpio::getADCreading(ADC_GPIO pin, int sampleCount)
{
    adc_channel_t channel;
    adc_unit_t adc_unit;
    if (adc_oneshot_io_to_channel(pin, &adc_unit, &channel) != ESP_OK)
    {
        log_e("getADCreading: GPIO %u is not ADC", pin);
        abort();
    }
    else if (sampleCount > 0)
    {
        adc_oneshot_unit_handle_t handle;
        adc_oneshot_unit_init_cfg_t unitCfg =
            {
                .unit_id = adc_unit,
                .ulp_mode = ADC_ULP_MODE_DISABLE,
            };
        adc_oneshot_chan_cfg_t channelCfg =
            {
                .atten = adc_atten_t::ADC_ATTEN_DB_12,
                .bitwidth = ADC_BITWIDTH_12,
            };
        ESP_ERROR_CHECK(adc_oneshot_new_unit(&unitCfg, &handle));
        ESP_ERROR_CHECK(adc_oneshot_config_channel(handle, channel, &channelCfg));
        int result = 0;
        for (int i = 0; i < sampleCount; i++)
        {
            int reading;
            ESP_ERROR_CHECK(adc_oneshot_read(handle, channel, &reading));
            result += reading;
        }
        result = result / sampleCount;
        ESP_ERROR_CHECK(adc_oneshot_del_unit(handle));
        return result;
    }
    return -1;
}

void internals::hal::gpio::forOutput(
    OutputGPIO pin,
    bool initialLevel,
    bool openDrain)
{
    if (!GPIO_IS_VALID_OUTPUT_GPIO((int)pin))
    {
        log_e("Requested GPIO %d can't be used as output", pin);
        abort();
    }
    else
    {
        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_DISABLE;
        if (openDrain)
            io_conf.mode = GPIO_MODE_OUTPUT_OD;
        else
            io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = (1ULL << (int)pin);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        ESP_ERROR_CHECK(gpio_config(&io_conf));
        gpio_set_level(AS_GPIO(pin), initialLevel);
    }
}

void internals::hal::gpio::forInput(
    InputGPIO pin,
    bool enablePullDown,
    bool enablePullUp)
{
    if (!GPIO_IS_VALID_GPIO((int)pin))
    {
        log_e("Requested GPIO %d can't be used as input", pin);
        abort();
    }
    else
    {
        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pin_bit_mask = (1ULL << (int)pin);
        if (enablePullDown)
            io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
        else
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        if (enablePullUp)
            io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        else
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        ESP_ERROR_CHECK(gpio_config(&io_conf));
    }
}

void internals::hal::gpio::enableISR(InputGPIO pin, ISRHandler handler, void *param)
{
    esp_err_t err = gpio_install_isr_service(0);
    if ((err != ESP_OK) && (err != ESP_ERR_INVALID_STATE))
        ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(gpio_set_intr_type(AS_GPIO(pin), GPIO_INTR_ANYEDGE));
    ESP_ERROR_CHECK(gpio_isr_handler_add(AS_GPIO(pin), handler, param));
    ESP_ERROR_CHECK(gpio_intr_enable(AS_GPIO(pin)));
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void internals::hal::gpio::wait_propagation(uint32_t nanoseconds)
{
    // This loop should be translated to 3 assembler instructions:
    // counter increase, no-operation and conditional jump.
    // So, each loop should take 3 CPU cycles.
    // Note: 1 ns = 1000 MHz
    static uint32_t loopTimeNs = ((getCpuFrequencyMhz() < 1000) ? (1000 / getCpuFrequencyMhz()) : 1)*3;
    for (uint32_t delay = 0; delay < nanoseconds; delay += loopTimeNs)
        __asm__ __volatile__(" nop\n");
}
#pragma GCC pop_options
