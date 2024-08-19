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
#include "i2c.h"
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
#define MAX17043_I2C_ADDRESS_SHIFTED 0x6c
static uint8_t fg_i2c_address;

// Other
#define DAEMON_STACK_SIZE 2048
static const char *MSG_INIT_ERROR = "batteryMonitor::begin(): unable to start daemon";

// ----------------------------------------------------------------------------
// Setters
// ----------------------------------------------------------------------------

void batteryMonitor::setPeriod(uint32_t seconds)
{
    if (seconds = 0)
        sampling_rate_ticks = DEFAULT_SAMPLING_RATE_TICKS;
    else
        sampling_rate_ticks = (seconds * 1000) / portTICK_RATE_MS;
}

// ----------------------------------------------------------------------------

void batteryMonitor::setWarningSoC(uint8_t percentage)
{
    if ((percentage >= 0) && (percentage <= 100))
    {
        low_battery_soc = percentage;
        // if (powerOff_soc > low_battery_soc)
        //     powerOff_soc = low_battery_soc;
    }
}

// ----------------------------------------------------------------------------

void batteryMonitor::setPowerOffSoC(uint8_t percentage)
{
    if ((percentage >= 0) && (percentage <= 100))
    {
        powerOff_soc = percentage;
        // if (powerOff_soc > low_battery_soc)
        //     low_battery_soc = powerOff_soc;
    }
}

// ----------------------------------------------------------------------------

void batteryMonitor::configureForTesting()
{
    setPeriod(5);
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

bool max17043_isPresent()
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    // Command VERSION = 0x0008. This command ensures the i2c slave
    // is a MAX17043 chip or compatible.
    uint16_t version;
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, fg_i2c_address | I2C_MASTER_READ, true);
    i2c_master_write_byte(cmd, 0x00, true);
    i2c_master_write_byte(cmd, 0x08, true);
    i2c_master_read(cmd, (uint8_t *)&version, sizeof(version), I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    bool result = (i2c_master_cmd_begin(I2C_NUM_0, cmd, DEBOUNCE_TICKS) == ESP_OK);
    i2c_cmd_link_delete(cmd);
    return result;
}

// ----------------------------------------------------------------------------

bool max17043_quickStart()
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    // Command QUICK START = 0x4000.
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, fg_i2c_address | I2C_MASTER_READ, true);
    i2c_master_write_byte(cmd, 0x40, true);
    i2c_master_write_byte(cmd, 0x00, true);
    i2c_master_stop(cmd);
    bool result = (i2c_master_cmd_begin(I2C_NUM_0, cmd, DEBOUNCE_TICKS) == ESP_OK);
    i2c_cmd_link_delete(cmd);
    return result;
}

// ----------------------------------------------------------------------------

bool max17043_getSoC(int &batteryLevel)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    // Command SOC = 0x0004
    uint16_t soc;
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, fg_i2c_address | I2C_MASTER_READ, true);
    i2c_master_write_byte(cmd, 0x00, true);
    i2c_master_write_byte(cmd, 0x04, true);
    i2c_master_read(cmd, (uint8_t *) &soc, sizeof(soc), I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    bool result = (i2c_master_cmd_begin(I2C_NUM_0, cmd, DEBOUNCE_TICKS) == ESP_OK);
    i2c_cmd_link_delete(cmd);
    if (result)
    {
        batteryLevel = (soc >> 8);
    }
    return result;
}

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
        uint64_t sum = 0;
        for (int i = 0; i < VOLTAGE_SAMPLES_COUNT; i++)
        {
            // sum += analogRead(batteryREADPin);
            sum += getADCreading(battREADPin, ADC_ATTEN_DB_11);
            // vTaskDelay(10);
        }
        if (battENPin >= 0)
            gpio_set_level(battENPin, 0);
        return (sum / VOLTAGE_SAMPLES_COUNT);
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
        log_e("power::startBatteryMonitor(): given pins are not usable");
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
    else
    {
        // Battery(+) is not connected, so battery level is unknown
        batteryLevel = UNKNOWN_BATTERY_LEVEL;
    }
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
            // Use fuel gauge
            ok = max17043_getSoC(lastBatteryLevel);
        else
            // Use battery monitor
            ok = batteryMonitor_getSoC(lastBatteryLevel);

        if (!ok)
            lastBatteryLevel = UNKNOWN_BATTERY_LEVEL;

        // Report battery level
        hidImplementation::reportBatteryLevel(lastBatteryLevel);
        if (ok && (lastBatteryLevel <= powerOff_soc))
        {
            // The DevKit must go to deep sleep before battery depletes, otherwise, it keeps
            // draining current even if there is not enough voltage to turn it on.
            power::powerOff();
        }
        else if (ok && (lastBatteryLevel <= low_battery_soc))
            notify::lowBattery();

        // Delay to next sample
        vTaskDelay(sampling_rate_ticks);

    } // end while
}

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void batteryMonitor::begin(gpio_num_t battENPin, gpio_num_t battREADPin)
{
    if (batteryMonitorDaemon == nullptr)
    {
        configureBatteryMonitor(battENPin, battREADPin);
        batteryCalibration::begin()
        xTaskCreate(
            batteryMonitorDaemonLoop,
            "BattMon",
            DAEMON_STACK_SIZE,
            nullptr,
            tskIDLE_PRIORITY + 1, &batteryMonitorDaemon);
        if (batteryMonitorDaemon == nullptr)
        {
            // log_e(MSG_INIT_ERROR);
            abort();
        }
    }
}

// ----------------------------------------------------------------------------

void batteryMonitor::begin(uint8_t i2c_address)
{
    i2c::abortOnInvalidAddress(i2c_address,0,0xFE);
    if (batteryMonitorDaemon == nullptr)
    {
        capabilities::setFlag(deviceCapability_t::CAP_BATTERY);
        fg_i2c_address = (i2c_address > 127) ? MAX17043_I2C_ADDRESS_SHIFTED : (i2c_address << 1);
        i2c::require();
        if (!max17043_isPresent())
        {
            log_w("Fuel gauge not found in the I2C bus");
            // note: no abort()
        }
        else
            max17043_quickStart();

        xTaskCreate(
            batteryMonitorDaemonLoop,
            "BattMonFG",
            DAEMON_STACK_SIZE,
            nullptr,
            tskIDLE_PRIORITY + 1, &batteryMonitorDaemon);
        if (batteryMonitorDaemon == nullptr)
        {
            // log_e(MSG_INIT_ERROR);
            abort();
        }
    }
}
