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
#include "BatteryMonitorHardware.hpp"

#if !CD_CI
#include "freertos/FreeRTOS.h"
#else
#include <iostream>
#include <thread>
#endif

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Globals
//-------------------------------------------------------------------
//-------------------------------------------------------------------

#define DEFAULT_SAMPLING_SECONDS (2 * 60)
static uint8_t low_battery_soc = 10;
static uint8_t powerOff_soc = 0;
static uint32_t sampling_rate_secs = DEFAULT_SAMPLING_SECONDS;
static BatteryMonitorInterface *batteryMonitorhardware = nullptr;
static BatteryStatus currentStatus;

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
    if (batteryMonitorhardware != nullptr)
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
    batteryMonitorhardware = new VoltageDividerMonitor(battREADPin, battENPin);
    DeviceCapabilities::setFlag(DeviceCapability::BATTERY);
}

void batteryMonitor::configure(I2CBus bus, uint8_t i2c_address)
{
    abortIfStarted();
    abortIfConfigured();
    batteryMonitorhardware = new MAX1704x(bus, i2c_address);
    DeviceCapabilities::setFlag(DeviceCapability::BATTERY);
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
// Internal service
//-------------------------------------------------------------------

class BatteryServiceProvider : public BatteryService
{
public:
    virtual int getLastBatteryLevel() override
    {
        return (int)currentStatus.stateOfCharge.value_or(UNKNOWN_BATTERY_LEVEL);
    }

    virtual bool hasBattery() override
    {
        return (batteryMonitorhardware != nullptr);
    }

    virtual bool isBatteryPresent() override
    {
        return (batteryMonitorhardware != nullptr) &&
               currentStatus.isBatteryPresent.value_or(false);
    }

    virtual void getStatus(BatteryStatus &status) override
    {
        status = currentStatus;
    }
};

//-------------------------------------------------------------------
// SoC daemon
//-------------------------------------------------------------------

void batteryMonitorDaemonLoop(void *arg)
{
    assert(batteryMonitorhardware != nullptr);
    BatteryStatus newBatteryStatus;
    while (true)
    {
        batteryMonitorhardware->getStatus(newBatteryStatus);
        uint8_t previousSoC = currentStatus.stateOfCharge.value_or(UNKNOWN_BATTERY_LEVEL);
        uint8_t newSoC = newBatteryStatus.stateOfCharge.value_or(UNKNOWN_BATTERY_LEVEL);

        if (previousSoC != newSoC)
            OnBatteryLevel::notify(newSoC);

        if (newBatteryStatus.stateOfCharge.has_value())
        {
            if ((powerOff_soc > 0) && (newSoC <= powerOff_soc))
            {
                // The DevKit must go to deep sleep before the battery depletes, otherwise, it keeps
                // draining current even if there is not enough voltage to turn it on.
                PowerService::call::shutdown();
            }
            else if (newSoC <= low_battery_soc)
                OnLowBattery::notify();
        }

        currentStatus = newBatteryStatus;

        // Delay to next sample
        DELAY_MS(sampling_rate_secs * 1000);

    } // end while
}

//-------------------------------------------------------------------

void batteryMonitorStart()
{
    if ((!FirmwareService::call::isRunning()) && (batteryMonitorhardware != nullptr))
    {
        OnBatteryLevel::notify(BatteryService::call::getLastBatteryLevel());
        batteryMonitorhardware->onStart();
#if !CD_CI
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
    if ((!FirmwareService::call::isRunning()) && (batteryMonitorhardware != nullptr))
    {
        BatteryService::inject(new BatteryServiceProvider());
        // Ensure there is a first reading available before the OnStart event
        batteryMonitorhardware->getStatus(currentStatus);
        OnStart::subscribe(batteryMonitorStart);
    }
}

// ----------------------------------------------------------------------------

void internals::batteryMonitor::configureFakeMonitor(BatteryStatus *fakeStatus)
{
    abortIfStarted();
    abortIfConfigured();
    batteryMonitorhardware = new FakeBatteryMonitor(fakeStatus);
    DeviceCapabilities::setFlag(DeviceCapability::BATTERY);
}

// ----------------------------------------------------------------------------

void *internals::batteryMonitor::getHardwareInstance()
{
    return static_cast<void *>(batteryMonitorhardware);
}