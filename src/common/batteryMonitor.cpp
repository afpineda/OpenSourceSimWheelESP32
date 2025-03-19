/**
 * @file batteryMonitor.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-08-16
 * @brief Everything related to the measurement of available battery charge.
 *
 * @copyright Licensed under the EUPL
 *
 */

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "HAL.hpp"
#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"

#if !CD_CI
#include "freertos/FreeRTOS.h"
#include "driver/i2c.h" // For i2c operation
#define I2C_WAIT_TICKS pdMS_TO_TICKS(150)
#else
#include <iostream>
#include <thread>
#endif

//-------------------------------------------------------------------
// Globals
//-------------------------------------------------------------------

// Global parameters
#define DEFAULT_SAMPLING_SECONDS (2 * 60)
static uint8_t low_battery_soc = 10;
static uint8_t powerOff_soc = 4;
int lastBatteryLevel = 100;
bool configured = false;
uint32_t sampling_rate_secs = DEFAULT_SAMPLING_SECONDS;
int lastBatteryReading = 0;

// Battery monitor
static OutputGPIO _batteryENPin;
static ADC_GPIO _batteryREADPin;
#define VOLTAGE_SAMPLES_COUNT 100

// Fuel gauge
#define MAX1704x_I2C_ADDRESS_SHIFTED 0x6c
#define MAX1704x_REG_SoC 0x04
#define MAX1704x_REG_MODE 0x06
#define MAX1704x_REG_VERSION 0x08
#define MAX1704x_REG_CONFIG 0x0C
static uint8_t fg_i2c_address = 0xFF; // 8-bit format

// Other
#define DAEMON_STACK_SIZE 2512

//-------------------------------------------------------------------
// Auxiliary
//-------------------------------------------------------------------

static void abortIfStarted()
{
    if (FirmwareService::call::isRunning())
        throw std::runtime_error("Battery monitor already started");
}

static void abortIfConfigured()
{
    if (configured)
        throw std::runtime_error("Battery monitor already configured");
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Public
//-------------------------------------------------------------------
//-------------------------------------------------------------------

void batteryMonitor::configure(ADC_GPIO battREADPin, OutputGPIO battENPin)
{
    abortIfStarted();
    abortIfConfigured();
    battREADPin.reserve();
    internals::hal::gpio::forInput(battREADPin, false, false);
    _batteryREADPin = battREADPin;
    DeviceCapabilities::setFlag(DeviceCapability::BATTERY);
    _batteryENPin = battENPin;
    if (battENPin != UNSPECIFIED::VALUE)
    {
        battENPin.reserve();
        internals::hal::gpio::forOutput(battENPin, false, false);
    }
    configured = true;
}

void batteryMonitor::configure(I2CBus bus, uint8_t i2c_address)
{
    abortIfStarted();
    abortIfConfigured();
    if (i2c_address < 128)
    {
        internals::hal::i2c::abortOnInvalidAddress(i2c_address);
        fg_i2c_address = (i2c_address << 1);
    }
    else
        fg_i2c_address = MAX1704x_I2C_ADDRESS_SHIFTED;
    internals::hal::i2c::require(4, bus);
    DeviceCapabilities::setFlag(DeviceCapability::BATTERY);
    configured = true;
}

void batteryMonitor::setPeriod(uint32_t seconds)
{
    if (seconds == 0)
        sampling_rate_secs = DEFAULT_SAMPLING_SECONDS;
    else
        sampling_rate_secs = seconds;
}

void batteryMonitor::setWarningSoC(uint8_t percentage)
{
    if (percentage <= 100)
    {
        low_battery_soc = percentage;
        // if (powerOff_soc > low_battery_soc)
        //     powerOff_soc = low_battery_soc;
    }
}

void batteryMonitor::setPowerOffSoC(uint8_t percentage)
{
    if (percentage <= 100)
    {
        powerOff_soc = percentage;
        // if (powerOff_soc > low_battery_soc)
        //     low_battery_soc = powerOff_soc;
    }
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Internals
//-------------------------------------------------------------------
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// Fuel gauge commands
//-------------------------------------------------------------------

bool max1704x_read(uint8_t regAddress, uint16_t &value)
{
#if !CD_CI
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, fg_i2c_address | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, regAddress, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, fg_i2c_address | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, ((uint8_t *)&value) + 1, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, ((uint8_t *)&value), I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    bool result = (i2c_master_cmd_begin(I2C_NUM_0, cmd, I2C_WAIT_TICKS) == ESP_OK);
    i2c_cmd_link_delete(cmd);
    return result;
#else
    return false;
#endif
}

//-------------------------------------------------------------------

bool max1704x_write(uint8_t regAddress, uint16_t value)
{
#if !CD_CI
    uint8_t *data = (uint8_t *)&value;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, fg_i2c_address | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, regAddress, true);
    i2c_master_write_byte(cmd, data[1], true);
    i2c_master_write_byte(cmd, data[0], true);
    i2c_master_stop(cmd);
    bool result = (i2c_master_cmd_begin(I2C_NUM_0, cmd, I2C_WAIT_TICKS) == ESP_OK);
    i2c_cmd_link_delete(cmd);
    return result;
#else
    return false;
#endif
}

//-------------------------------------------------------------------

bool max1704x_quickStart()
{
#if !CD_CI
    return max1704x_write(MAX1704x_REG_MODE, 0x4000);
#else
    return false;
#endif
}

//-------------------------------------------------------------------

bool max1704x_getSoC(int &batteryLevel)
{
#if !CD_CI
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
#else
    return false;
#endif
}

//-------------------------------------------------------------------

// bool max1704x_isPresent()
// {
// #if !CD_CI
//     // Command VERSION. This command ensures the i2c slave
//     // is a MAX1704x chip or compatible.
//     uint16_t version;
//     bool result = max1704x_read(MAX1704x_REG_VERSION, version);
//     if (result)
//         log_i("Fuel gauge version: %d", version);
//     return result;
// #else
//     return false;
// #endif
// }

//-------------------------------------------------------------------

// bool max1704x_getCompensation(uint8_t &compensation)
// {
//     uint16_t currentConfig;
//     bool result = max1704x_read(MAX1704x_REG_CONFIG, currentConfig);
//     if (result)
//         compensation = ((uint8_t *)&currentConfig)[1];
//     return result;
// }

//-------------------------------------------------------------------

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

//-------------------------------------------------------------------
// Battery monitor/voltage divider commands
//-------------------------------------------------------------------

bool batteryMonitor_getSoC(int &batteryLevel)
{
#if !CD_CI
    // Enable circuit when required
    if (_batteryENPin != UNSPECIFIED::VALUE)
    {
        GPIO_SET_LEVEL(_batteryENPin, 1);
        DELAY_TICKS(200);
    }
#endif

    // Get ADC reading
    // Note: lastBatteryReading is global to allow testing
    lastBatteryReading = internals::hal::gpio::getADCreading(_batteryREADPin, VOLTAGE_SAMPLES_COUNT);

#if !CD_CI
    // Disable circuit when required
    if (_batteryENPin != UNSPECIFIED::VALUE)
    {
        GPIO_SET_LEVEL(_batteryENPin, 0);
    }
#endif

    bool result = (lastBatteryReading >= 150);
    if (result)
    {
        batteryLevel =
            BatteryCalibrationService::call::getBatteryLevel(lastBatteryReading);
        if (batteryLevel < 0)
        {
            // Battery calibration is *not* available
            // fallback to auto-calibration algorithm
            batteryLevel =
                BatteryCalibrationService::call::getBatteryLevelAutoCalibrated(lastBatteryReading);
        }
    }
    // else
    //  Battery(+) is not connected, so battery level is unknown

    return result;
}

//-------------------------------------------------------------------
// SoC daemon
//-------------------------------------------------------------------

void batteryMonitorDaemonLoop(void *arg)
{
    while (true)
    {
        bool ok;
        int currentBatteryLevel;
        if (_batteryREADPin == UNSPECIFIED::VALUE)
        {
            // Use fuel gauge
            ok = max1704x_getSoC(currentBatteryLevel);
        }
        else
        {
            // Use battery monitor
            ok = batteryMonitor_getSoC(currentBatteryLevel);
        }

        // Report battery level
        if (!ok)
            currentBatteryLevel = UNKNOWN_BATTERY_LEVEL;

        if (currentBatteryLevel != lastBatteryLevel)
        {
            lastBatteryLevel = currentBatteryLevel;
            OnBatteryLevel::notify(lastBatteryLevel);
        }

        // Notifications and shutdown
        if (ok)
        {
            if ((powerOff_soc > 0) && (lastBatteryLevel <= powerOff_soc))
            {
                // The DevKit must go to deep sleep before battery depletes, otherwise, it keeps
                // draining current even if there is not enough voltage to turn it on.
                PowerService::call::shutdown();
            }
            else if (lastBatteryLevel <= low_battery_soc)
                OnLowBattery::notify();
        }

        // Delay to next sample
        DELAY_MS(sampling_rate_secs * 1000);

    } // end while
}

//-------------------------------------------------------------------
// Internal service
//-------------------------------------------------------------------

class BatteryServiceProvider : public BatteryService
{
public:
    virtual int getLastBatteryLevel() override
    {
        return lastBatteryLevel;
    }
};

//-------------------------------------------------------------------

void batteryMonitorStart()
{
    if ((!FirmwareService::call::isRunning()) && (configured))
    {
        OnBatteryLevel::notify(lastBatteryLevel);
#if !CD_CI
        if (_batteryREADPin == UNSPECIFIED::VALUE)
            max1704x_quickStart();
        TaskHandle_t batteryMonitorDaemon = nullptr;
        xTaskCreate(
            batteryMonitorDaemonLoop,
            "BattMon",
            DAEMON_STACK_SIZE,
            nullptr,
            tskIDLE_PRIORITY + 1, &batteryMonitorDaemon);
        if (!batteryMonitorDaemon)
            throw std::runtime_error("Unable to start the battery monitor daemon");
#else
        std::jthread daemon(batteryMonitorDaemonLoop, nullptr);
        daemon.detach();
#endif
    }
}

// ----------------------------------------------------------------------------

void internals::batteryMonitor::configureForTesting()
{
    ::batteryMonitor::setPeriod(10);
    ::batteryMonitor::setPowerOffSoC(0);
    ::batteryMonitor::setWarningSoC(50);
}

// ----------------------------------------------------------------------------

void internals::batteryMonitor::getReady()
{
    if ((!FirmwareService::call::isRunning()) && (configured))
    {
        BatteryService::inject(new BatteryServiceProvider());
        OnStart::subscribe(batteryMonitorStart);
    }
}