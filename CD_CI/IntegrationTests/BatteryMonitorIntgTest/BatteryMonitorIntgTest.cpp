/**
 * @file BatteryMonitorIntgTest.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-08-23
 * @brief Integration test
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

BatteryStatus currentStatus;
int receivedBatteryLevel;
bool lowBattWitness = false;
bool shutdownWitness = false;
std::counting_semaphore<1> received{1};

//------------------------------------------------------------------
// MOCKS
//------------------------------------------------------------------

class PowerServiceMock : public PowerService
{
public:
    virtual void shutdown() override
    {
        OnShutdown::notify();
    }
};

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

void reset()
{
    currentStatus.isBatteryPresent.reset();
    currentStatus.isCharging.reset();
    currentStatus.stateOfCharge.reset();
    currentStatus.isBatteryPresent.reset();
    lowBattWitness = false;
    shutdownWitness = false;
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
// Auxiliary (event handlers)
//------------------------------------------------------------------

void get_current_battery_level(int level)
{
    // std::cout << "get_current_battery_level" << std::endl;
    receivedBatteryLevel = level;
    received.release();
}

void on_low_battery()
{
    lowBattWitness = true;
    received.release();
}

void on_shutdown()
{
    shutdownWitness = true;
    received.release();
}

//------------------------------------------------------------------
//------------------------------------------------------------------
// Test groups
//------------------------------------------------------------------
//------------------------------------------------------------------

void test1()
{
    std::cout << "- test 1 -" << std::endl;

    reset();
    currentStatus.stateOfCharge = 9;
    DELAY_MS(1500);
    waitFor("low battery warning");
    assert<bool>::equals("low battery witness (1)", true, lowBattWitness);

    reset();
    currentStatus.stateOfCharge = 1;
    DELAY_MS(1500);
    waitFor("critical battery level");
    assert<bool>::equals("shutdown on low battery (1)", true, shutdownWitness);

    reset();
    currentStatus.stateOfCharge = 99;
    DELAY_MS(1500);
    waitFor("fully charged battery");
    assert<bool>::equals("low battery witness (2)", false, lowBattWitness);
    assert<bool>::equals("shutdown on low battery (2)", false, shutdownWitness);

    reset();
    DELAY_MS(1500);
    waitFor("State of charge unknown");
    assert<bool>::equals("low battery witness (3)", false, lowBattWitness);
    assert<bool>::equals("shutdown on low battery (3)", false, shutdownWitness);
}

void test2()
{
    std::cout << "- test 2 -" << std::endl;

    reset();
    currentStatus.stateOfCharge = 33;
    DELAY_MS(1500);
    waitFor("State of charge 33");
    assert<bool>::equals("state of charge (1)", 33, BatteryService::call::getLastBatteryLevel());

    reset();
    DELAY_MS(1500);
    waitFor("State of charge unknown");
    assert<bool>::equals("state of charge (2)", UNKNOWN_BATTERY_LEVEL, BatteryService::call::getLastBatteryLevel());
}

//------------------------------------------------------------------
//------------------------------------------------------------------
// Entry point
//------------------------------------------------------------------
//------------------------------------------------------------------

int main()
{
    PowerService::inject(new PowerServiceMock);
    OnBatteryLevel::subscribe(get_current_battery_level);
    OnLowBattery::subscribe(on_low_battery);
    OnShutdown::subscribe(on_shutdown);
    internals::batteryMonitor::configureFakeMonitor(&currentStatus);
    batteryMonitor::setPeriod(1);
    batteryMonitor::setWarningSoC(10);
    batteryMonitor::setPowerOffSoC(4);
    internals::batteryMonitor::getReady();
    OnStart::notify();

    test1();
    test2();
}
