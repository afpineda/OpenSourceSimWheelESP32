/**
 * @file BatteryMonitorIntgTest.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-03-06
 * @brief Integration test
 *
 * @copyright Licensed under the EUPL
 *
 */

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include "cd_ci_assertions.hpp"
#include "HAL.hpp"
#include <cinttypes>
#include <cassert>
#include <iostream>

//-------------------------------------------------------------------
// Mocks
//-------------------------------------------------------------------

class PowerMock : public PowerService
{
public:
    bool powerOff = false;
    virtual void shutdown() override
    {
        powerOff = true;
    }
} powerMock;

class BatteryCalMock : public BatteryCalibrationService
{
public:
    virtual int getBatteryLevel(int reading) override
    {
        return reading / 41;
    }
} battCalMock;

bool lowBattWarning = false;

void lowBattCallback()
{
    lowBattWarning = true;
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------
//-------------------------------------------------------------------

int main()
{
    PowerService::inject(&powerMock);
    BatteryCalibrationService::inject(&battCalMock);
    OnLowBattery::subscribe(lowBattCallback);
    batteryMonitor::configure(0);
    batteryMonitor::setPeriod(1);
    batteryMonitor::setPowerOffSoC(20);
    batteryMonitor::setWarningSoC(50);
    internals::hal::gpio::fakeADCreading = 4096;
    internals::batteryMonitor::getReady();
    OnStart::notify();

    // Check initial state
    DELAY_MS(1100);
    uint8_t batteryLevel = BatteryService::call::getLastBatteryLevel();
    assert<uint8_t>::equals("InitialState", 99, batteryLevel);

    // Low battery reading
    internals::hal::gpio::fakeADCreading = 1800;
    DELAY_MS(1100);
    batteryLevel = BatteryService::call::getLastBatteryLevel();
    assert<int>::equals("Low battery state", 43, batteryLevel);
    assert<bool>::equals("Low battery notification", true, lowBattWarning);

    // Critical battery level
    internals::hal::gpio::fakeADCreading = 300;
    DELAY_MS(1100);
    batteryLevel = BatteryService::call::getLastBatteryLevel();
    assert<int>::equals("Critical battery state", 7, batteryLevel);
    assert<bool>::equals("Automatic shutdown", true, powerMock.powerOff);
}
