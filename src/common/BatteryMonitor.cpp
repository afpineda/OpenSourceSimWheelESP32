/**
 * @file BatteryMonitor.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-08-16
 * @brief  Implementation of the `batteryMonitor` namespace
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.h"
#include "adcTools.h"
#include "esp32-hal-gpio.h"
#include "i2cTools.h"
#include "driver/i2c.h"

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

// Global parameters
#define DEFAULT_SAMPLING_RATE_TICKS ((2 * 60 * 1000) / portTICK_RATE_MS) // 2 minutes
static TickType_t sampling_rate_ticks = DEFAULT_SAMPLING_RATE_TICKS;
static uint8_t low_battery_soc = 10;
static uint8_t powerOff_soc = 4;
int lastBatteryLevel = 100;

// Battery monitor
static gpio_num_t batteryENPin = (gpio_num_t)-1;
static gpio_num_t batteryREADPin = (gpio_num_t)-1;
static TaskHandle_t batteryMonitorDaemon = nullptr;
#define VOLTAGE_SAMPLES_COUNT 100

// Fuel gauge
#define MAX1704x_I2C_ADDRESS_SHIFTED 0x6c
#define MAX1704x_REG_SoC 0x04
#define MAX1704x_REG_MODE 0x06
#define MAX1704x_REG_VERSION 0x08
#define MAX1704x_REG_CONFIG 0x0C
static uint8_t fg_i2c_address;

// Other
#define DAEMON_STACK_SIZE 2512

// ----------------------------------------------------------------------------
// Setters
// ----------------------------------------------------------------------------

void batteryMonitor::setPeriod(uint32_t seconds)
{
    if (seconds == 0)
        sampling_rate_ticks = DEFAULT_SAMPLING_RATE_TICKS;
    else
        sampling_rate_ticks = (seconds * 1000) / portTICK_RATE_MS;
}

// ----------------------------------------------------------------------------

void batteryMonitor::setWarningSoC(uint8_t percentage)
{
    if (percentage <= 100)
    {
        low_battery_soc = percentage;
        // if (powerOff_soc > low_battery_soc)
        //     powerOff_soc = low_battery_soc;
    }
}

// ----------------------------------------------------------------------------

void batteryMonitor::setPowerOffSoC(uint8_t percentage)
{
    if (percentage <= 100)
    {
        powerOff_soc = percentage;
        // if (powerOff_soc > low_battery_soc)
        //     low_battery_soc = powerOff_soc;
    }
}

// ----------------------------------------------------------------------------

void batteryMonitor::configureForTesting()
{
    setPeriod(10);
    setPowerOffSoC(0);
    setWarningSoC(50);
}

// ----------------------------------------------------------------------------
// Getters
// ----------------------------------------------------------------------------

int batteryMonitor::getLastBatteryLevel()
{
    return lastBatteryLevel;
}

// ----------------------------------------------------------------------------
// Fuel gauge commands
// ----------------------------------------------------------------------------

bool max1704x_read(uint8_t regAddress, uint16_t &value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, fg_i2c_address | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, regAddress, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, fg_i2c_address | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, ((uint8_t *)&value) + 1, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, ((uint8_t *)&value), I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    bool result = (i2c_master_cmd_begin(I2C_NUM_0, cmd, DEBOUNCE_TICKS) == ESP_OK);
    i2c_cmd_link_delete(cmd);
    return result;
}

// ----------------------------------------------------------------------------

bool max1704x_write(uint8_t regAddress, uint16_t value)
{
    uint8_t *data = (uint8_t *)&value;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, fg_i2c_address | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, regAddress, true);
    i2c_master_write_byte(cmd, data[1], true);
    i2c_master_write_byte(cmd, data[0], true);
    i2c_master_stop(cmd);
    bool result = (i2c_master_cmd_begin(I2C_NUM_0, cmd, DEBOUNCE_TICKS) == ESP_OK);
    i2c_cmd_link_delete(cmd);
    return result;
}

// ----------------------------------------------------------------------------

bool max1704x_isPresent()
{
    // Command VERSION. This command ensures the i2c slave
    // is a MAX1704x chip or compatible.
    uint16_t version;
    bool result = max1704x_read(MAX1704x_REG_VERSION, version);
    if (result)
        log_i("Fuel gauge version: %d", version);
    return result;
}

// ----------------------------------------------------------------------------

bool max1704x_quickStart()
{
    return max1704x_write(MAX1704x_REG_MODE, 0x4000);
}

// ----------------------------------------------------------------------------

bool max1704x_getSoC(int &batteryLevel)
{
    uint16_t value;
    bool result = max1704x_read(MAX1704x_REG_SoC, value);
    if (result)
    {
        uint8_t *data = (uint8_t *)&value;
        batteryLevel = data[1];
        if (data[0] >= 127)
            batteryLevel++;
    }
    return result;
}

// ----------------------------------------------------------------------------

// bool max1704x_getCompensation(uint8_t &compensation)
// {
//     uint16_t currentConfig;
//     bool result = max1704x_read(MAX1704x_REG_CONFIG, currentConfig);
//     if (result)
//         compensation = ((uint8_t *)&currentConfig)[1];
//     return result;
// }

// // ----------------------------------------------------------------------------

// bool max1704x_setCompensation(uint8_t compensation)
// {
//     uint16_t currentConfig;
//     bool result = max1704x_read(MAX1704x_REG_CONFIG, currentConfig);
//     if (result)
//     {
//         uint8_t *pCurrentConfig = ((uint8_t *)&currentConfig);
//         pCurrentConfig[1] = compensation;
//         result = max1704x_write(MAX1704x_REG_CONFIG, currentConfig);
//     }
//     return result;
// }

// ----------------------------------------------------------------------------
// Battery monitor/voltage divider commands
// ----------------------------------------------------------------------------

/**
 * @brief Get ADC reading of the battery pin for testing purposes.
 *
 * @param battENPin Output-capable GPIO pin to enable battery readings.
 *                  Must have been properly initialized.
 * @param battREADPin ADC pin for reading. Must have been properly initialized.
 * @return int ADC reading
 */
int getBatteryReadingForTesting(gpio_num_t battENPin, gpio_num_t battREADPin)
{
    if ((battENPin < 0) || (gpio_set_level(battENPin, 1) == ESP_OK))
    {
        vTaskDelay(200);
        int mean = getADCreading(battREADPin, ADC_ATTEN_DB_12, VOLTAGE_SAMPLES_COUNT);
        if (battENPin >= 0)
            gpio_set_level(battENPin, 0);
        return mean;
    }
    return 0;
}

// ----------------------------------------------------------------------------

void configureBatteryMonitor(
    gpio_num_t enableBatteryReadPin,
    gpio_num_t batteryLevelPin)
{
    if (!GPIO_IS_VALID_GPIO(batteryLevelPin) ||
        ((enableBatteryReadPin >= 0) && !GPIO_IS_VALID_OUTPUT_GPIO(enableBatteryReadPin)) ||
        (digitalPinToAnalogChannel(batteryLevelPin) < 0))
    {
        log_e("batteryMonitor::begin(): given pins are not usable");
        abort();
    }

    gpio_config_t io_conf = {};
    if (enableBatteryReadPin >= 0)
    {
        // configure _battEN_ pin
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = (1ULL << enableBatteryReadPin);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        ESP_ERROR_CHECK(gpio_config(&io_conf));
        ESP_ERROR_CHECK(gpio_set_level(enableBatteryReadPin, 0));
    }

    // configure _battRead_ pin
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << batteryLevelPin);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // Store parameters
    batteryENPin = enableBatteryReadPin;
    batteryREADPin = batteryLevelPin;

    capabilities::setFlag(deviceCapability_t::CAP_BATTERY);
}

// ----------------------------------------------------------------------------

bool batteryMonitor_getSoC(int &batteryLevel)
{
    // Determine battery level
    int lastBatteryReading = getBatteryReadingForTesting(batteryENPin, batteryREADPin);
    bool result = (lastBatteryReading >= 150);
    if (result)
    {
        batteryLevel = batteryCalibration::getBatteryLevel(lastBatteryReading);
        if (batteryLevel < 0)
        {
            // Battery calibration is *not* available
            // fallback to auto-calibration algorithm
            batteryLevel = batteryCalibration::getBatteryLevelAutoCalibrated(lastBatteryReading);
        }
    }
    // else
    //     // Battery(+) is not connected, so battery level is unknown
    return result;
}

// ----------------------------------------------------------------------------
// SoC daemon
// ----------------------------------------------------------------------------

void batteryMonitorDaemonLoop(void *arg)
{
    while (true)
    {
        bool ok;
        if (batteryREADPin < 0)
        {
            // Use fuel gauge
            ok = max1704x_getSoC(lastBatteryLevel);
        }
        else
        {
            // Use battery monitor
            ok = batteryMonitor_getSoC(lastBatteryLevel);
        }

        // Report battery level
        if (!ok)
            lastBatteryLevel = UNKNOWN_BATTERY_LEVEL;
        hidImplementation::reportBatteryLevel(lastBatteryLevel);

        // notifications and power-off
        if (ok)
        {
            if ((powerOff_soc > 0) && (lastBatteryLevel <= powerOff_soc))
            {
                // The DevKit must go to deep sleep before battery depletes, otherwise, it keeps
                // draining current even if there is not enough voltage to turn it on.
                power::powerOff();
            }
            else if (lastBatteryLevel <= low_battery_soc)
                notify::lowBattery();
        }

        // Delay to next sample
        //        log_i("Sleep %lld",sampling_rate_ticks);
        vTaskDelay(sampling_rate_ticks);

    } // end while
}

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void batteryMonitor_startDaemon()
{
    xTaskCreate(
        batteryMonitorDaemonLoop,
        "BattMon",
        DAEMON_STACK_SIZE,
        nullptr,
        tskIDLE_PRIORITY + 1, &batteryMonitorDaemon);
    if (batteryMonitorDaemon == nullptr)
    {
        log_e("batteryMonitor::begin(): unable to start daemon");
        abort();
    }
}

// ----------------------------------------------------------------------------

void batteryMonitor::begin(gpio_num_t battENPin, gpio_num_t battREADPin)
{
    if (batteryMonitorDaemon == nullptr)
    {
        configureBatteryMonitor(battENPin, battREADPin);
        batteryCalibration::begin();
        batteryMonitor_startDaemon();
    }
}

// ----------------------------------------------------------------------------

void batteryMonitor::begin(uint8_t i2c_address)
{
    if (i2c_address != 0xFF)
        i2c::abortOnInvalidAddress(i2c_address);
    if (batteryMonitorDaemon == nullptr)
    {
        capabilities::setFlag(deviceCapability_t::CAP_BATTERY);
        fg_i2c_address = (i2c_address > 127) ? MAX1704x_I2C_ADDRESS_SHIFTED : (i2c_address << 1);
        i2c::require();
        batteryMonitor_startDaemon();
    }
}