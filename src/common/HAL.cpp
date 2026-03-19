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
#include <array>
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
static i2c_master_bus_config_t i2c_master_bus_cfg[] = {
    {

        .i2c_port = 0,
        .sda_io_num = (gpio_num_t)SDA,
        .scl_io_num = (gpio_num_t)SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags{
            .enable_internal_pullup = 1,
            .allow_pd = 0,
        },
    },
    {
        .i2c_port = 1,
        .sda_io_num = (gpio_num_t)GPIO_NUM_NC,
        .scl_io_num = (gpio_num_t)GPIO_NUM_NC,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags{
            .enable_internal_pullup = 1,
            .allow_pd = 0,
        },
    },
};
static i2c_master_bus_handle_t i2c_bus_handle[] =
    {nullptr, nullptr};

// ADC
static std::array<adc_oneshot_unit_handle_t, SOC_ADC_PERIPH_NUM> adc_handler{nullptr};
static uint64_t initialized_adc_pins = 0ULL; // A bitmap

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// I2C
//-------------------------------------------------------------------
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// I2C: Probe
//-------------------------------------------------------------------

bool internals::hal::i2c::probe(uint8_t address7bits, I2CBus bus)
{
    internals::hal::i2c::abortOnInvalidAddress(address7bits);
    if (i2c_bus_handle[(int)(bus)])
    {
        esp_err_t err = i2c_master_probe(
            i2c_bus_handle[(int)(bus)],
            address7bits,
            pdMS_TO_TICKS(150));
        return (err == ESP_OK);
    }
    else
        throw i2c_error(
            i2c_master_bus_cfg[(int)(bus)].sda_io_num,
            i2c_master_bus_cfg[(int)(bus)].scl_io_num,
            (int)bus);
    return false;
}

// ----------------------------------------------------------------------------

void internals::hal::i2c::probe(std::vector<uint8_t> &result, I2CBus bus)
{
    result.clear();
    internals::hal::i2c::require(bus);

    // Probe
    for (uint8_t address = 0; address < 128; address++)
    {
        if (internals::hal::i2c::probe(address, bus))
            result.push_back(address);
    }
}

// ----------------------------------------------------------------------------
// I2C: bus initialization
// ----------------------------------------------------------------------------

void internals::hal::i2c::initialize(
    GPIO sda,
    GPIO scl,
    I2CBus bus,
    bool enableInternalPullup)
{
    i2c_master_bus_cfg[(int)(bus)].sda_io_num = AS_GPIO(sda);
    i2c_master_bus_cfg[(int)(bus)].scl_io_num = AS_GPIO(scl);
    i2c_master_bus_cfg[(int)(bus)].flags.enable_internal_pullup =
        (enableInternalPullup) ? 1 : 0;
}

void internals::hal::i2c::require(I2CBus bus)
{
    if (!i2c_bus_handle[(int)(bus)])
    {
        // Not used before
        if (i2c_new_master_bus(
                &i2c_master_bus_cfg[(int)(bus)],
                &i2c_bus_handle[(int)(bus)]) != ESP_OK)
            throw i2c_error(
                i2c_master_bus_cfg[(int)(bus)].sda_io_num,
                i2c_master_bus_cfg[(int)(bus)].scl_io_num,
                (int)bus);
    }
}

// ----------------------------------------------------------------------------
// I2C: slave devices
// ----------------------------------------------------------------------------

i2c_master_dev_handle_t internals::hal::i2c::add_device(
    uint8_t address7bits,
    uint8_t max_speed_multiplier,
    I2CBus bus)
{
    if (max_speed_multiplier < 1)
        max_speed_multiplier = 1;
    if (max_speed_multiplier > 4)
        max_speed_multiplier = 4;
    internals::hal::i2c::require(bus);
    i2c_device_config_t cfg{
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address7bits,
        .scl_speed_hz = 100000 * (uint32_t)max_speed_multiplier,
        .scl_wait_us = 0, // use default
        .flags{
            .disable_ack_check = false,
        },
    };
    i2c_master_dev_handle_t result = nullptr;
    ESP_ERROR_CHECK(
        i2c_master_bus_add_device(
            i2c_bus_handle[(int)(bus)],
            &cfg,
            &result));
    return result;
}

void internals::hal::i2c::remove_device(i2c_master_dev_handle_t i2c_device)
{
    if (i2c_device)
        ESP_ERROR_CHECK(i2c_master_bus_rm_device(i2c_device));
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
        throw gpio_error(pin, "not ADC-capable");
    else if (sampleCount > 0)
    {
        if (!adc_handler[adc_unit])
        {
            // Initialize the required ADC unit
            adc_oneshot_unit_handle_t handle;
            adc_oneshot_unit_init_cfg_t unitCfg =
                {
                    .unit_id = adc_unit,
                    // From ESP-IDF doc: If set to 0, the driver will fall back to using a default clock source
                    .clk_src = static_cast<adc_oneshot_clk_src_t>(0),
                    .ulp_mode = ADC_ULP_MODE_DISABLE};
            handle = nullptr;
            ESP_ERROR_CHECK(adc_oneshot_new_unit(&unitCfg, &handle));
            if (!handle)
                throw std::runtime_error("getADCreading: adc_oneshot_new_unit() failed");
            adc_handler[adc_unit] = handle;

            // Note: adc_oneshot_del_unit(handle) is never called
        }

        if (~initialized_adc_pins & (1ULL << (uint8_t)pin))
        {
            // Configure this channel in the ADC unit
            // (only once)
            adc_oneshot_chan_cfg_t channelCfg =
                {
                    .atten = adc_atten_t::ADC_ATTEN_DB_12,
                    .bitwidth = ADC_BITWIDTH_12,
                };
            ESP_ERROR_CHECK(
                adc_oneshot_config_channel(
                    adc_handler[adc_unit],
                    static_cast<adc_channel_t>(channel),
                    &channelCfg));
            initialized_adc_pins |= (1ULL << pin);
        }

        int result = 0;
        for (int i = 0; i < sampleCount; i++)
        {
            int reading;
            esp_err_t err = adc_oneshot_read(adc_handler[adc_unit], channel, &reading);
            // DEV NOTE: in Arduino-ESP32 core version 3.2.1 and a "pure" ESP32 board,
            // adc_oneshot_read() returns ESP_ERR_TIMEOUT from time to time.
            // According to GitHub Copilot, the cause is the ADC2 unit being
            // shared with the BLE hardware.
            // Uncomment the following line for proof:
            // ESP_ERROR_CHECK_WITHOUT_ABORT(err);
            if (err == ESP_OK)
                result += reading;
            else
                // DEV NOTE: The following line could lead to an infinite loop in the worst case.
                // However, the result could be wrong if not executed.
                i--;
        }
        result = result / sampleCount;
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
