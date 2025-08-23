/**
 * @file VoltageDividerMonitorTest.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-23
 * @brief Unit test
 *
 * @copyright Licensed under the EUPL
 *
 */

//------------------------------------------------------------------
// Imports
//------------------------------------------------------------------

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalTypes.hpp"
#include "InternalServices.hpp"
#include "HAL.hpp"
#include "BatteryMonitorHardware.hpp"
#include "cd_ci_assertions.hpp"
#include <iostream>
#include <semaphore>
#include <chrono>

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

VoltageDividerMonitor *hardware = nullptr;
int receivedBatteryLevel = 999;
std::counting_semaphore<1> received{1};

//------------------------------------------------------------------
// MOCKS
//------------------------------------------------------------------

class BatteryCalibrationMock : public BatteryCalibrationService
{
public:
    virtual int getBatteryLevel(int reading) override
    {
        // std::cout << "getBatteryLevel" << std::endl;
        return reading / 100;
    }
};

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

void get_current_battery_level(int level)
{
    // std::cout << "get_current_battery_level" << std::endl;
    receivedBatteryLevel = level;
    received.release();
}

void waitFor(std::string message = "")
{
    if (!received.try_acquire_for(std::chrono::milliseconds(2000)))
    {
        std::cout << "Input event not received at: " << message << std::endl;
        assert(false && "Input event not received");
    }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
// Test groups
//------------------------------------------------------------------
//------------------------------------------------------------------

void test1()
{

    std::cout << "- test 1 -" << std::endl;
    internals::hal::gpio::setFakeADCReading({2000});
    DELAY_MS(2000);
    waitFor("1");
    assert<int>::equals("A", 2000, hardware->lastBatteryReading);
    assert<int>::equals("B", 20, receivedBatteryLevel);
}

void test2()
{
    BatteryStatus status;
    std::cout << "- test 2 (constant current charging simulation) -" << std::endl;
    internals::hal::gpio::setFakeADCReading({0, 1000, 2000, 3000, 4000, 4095, 4000, 3000, 2000, 1000});
    DELAY_MS(2000);
    waitFor("2");
    BatteryService::call::getStatus(status);
    assert<bool>::equals("known charging state", true, status.isCharging.has_value());
    assert<bool>::equals("charging state", true, status.isCharging.value());
    assert<bool>::equals("known wired power state", true, status.usingExternalPower.has_value());
    assert<bool>::equals("wired power state", true, status.usingExternalPower.value());
    assert<bool>::equals("known battery presence", true, status.isBatteryPresent.has_value());
    assert<bool>::equals("battery presence", true, status.isBatteryPresent.value());
    assert<bool>::equals("unknown SoC state", false, status.stateOfCharge.has_value());
}

void test3()
{
    BatteryStatus status;
    std::cout << "- test 3 (no battery) -" << std::endl;
    internals::hal::gpio::setFakeADCReading({20});
    DELAY_MS(2000);
    waitFor("3");
    BatteryService::call::getStatus(status);
    assert<bool>::equals("known charging state", true, status.isCharging.has_value());
    assert<bool>::equals("charging state", false, status.isCharging.value());
    assert<bool>::equals("known wired power state", true, status.usingExternalPower.has_value());
    assert<bool>::equals("wired power state", true, status.usingExternalPower.value());
    assert<bool>::equals("unknown SoC state", false, status.stateOfCharge.has_value());
    assert<bool>::equals("known battery presence", true, status.isBatteryPresent.has_value());
    assert<bool>::equals("battery presence", false, status.isBatteryPresent.value());
}

//------------------------------------------------------------------
//------------------------------------------------------------------
// Entry point
//------------------------------------------------------------------
//------------------------------------------------------------------

int main()
{
    BatteryCalibrationService::inject(new BatteryCalibrationMock());
    OnBatteryLevel::subscribe(get_current_battery_level);
    batteryMonitor::configure(TEST_RTC_GPIO1);
    hardware = static_cast<VoltageDividerMonitor *>(internals::batteryMonitor::getHardwareInstance());
    assert(hardware != nullptr);
    batteryMonitor::setPeriod(1);
    internals::batteryMonitor::getReady();
    OnStart::notify();

    test1();
    test2();
    test3();
}
