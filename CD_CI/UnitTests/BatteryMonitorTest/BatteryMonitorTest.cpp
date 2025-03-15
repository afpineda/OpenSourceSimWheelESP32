/**
 * @file BatteryMonitorTest.cpp
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
#include "cd_ci_assertions.hpp"
#include <iostream>
#include <semaphore>
#include <chrono>

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

extern int lastBatteryReading;
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
// Auxiliray
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
    internals::hal::gpio::fakeADCreading = 2000;
    DELAY_MS(2000);
    waitFor("1");
    assert<int>::equals("A", 2000, lastBatteryReading);
    assert<int>::equals("B", 20, receivedBatteryLevel);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
// Entry point
//------------------------------------------------------------------
//------------------------------------------------------------------

int main()
{
    internals::hal::gpio::fakeADCreading = 0;
    BatteryCalibrationService::inject(new BatteryCalibrationMock());
    OnBatteryLevel::subscribe(get_current_battery_level);
    batteryMonitor::configure(TEST_RTC_GPIO1);
    batteryMonitor::setPeriod(1);
    internals::batteryMonitor::getReady();
    OnStart::notify();

    test1();
}
