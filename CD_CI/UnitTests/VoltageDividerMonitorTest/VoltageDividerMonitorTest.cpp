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
bool running = false;
std::binary_semaphore producer{0};
std::binary_semaphore consumer{0};

BatteryStatus incoming_batt_status{};

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

void get_current_battery_level(const BatteryStatus &status)
{
    if (!running)
        return;
    producer.acquire();
    // std::cout << "get_current_battery_level" << std::endl;
    incoming_batt_status = status;
    consumer.release();
}

// void waitFor(std::string message = "")
// {
//     producer.release();
//     consumer.acquire();
// }

BatteryStatus waitFor()
{
    producer.release();
    consumer.acquire();
    BatteryStatus result = incoming_batt_status;
    return result;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
// Test groups
//------------------------------------------------------------------
//------------------------------------------------------------------

void test1()
{
    std::cout << "- test 1 (ADC value injection)-" << std::endl;
    // NOTE: already called in main before OnStart()
    // otherwise it fails on linux.
    //   internals::hal::gpio::setFakeADCReading({2000});

    BatteryStatus status = waitFor();
    assert<int>::equals("A", 2000, hardware->lastBatteryReading);
    assert<int>::equals("B", 20, status.stateOfCharge.value_or(20));
}

void test2()
{
    std::cout << "- test 2 (constant current charging simulation) -" << std::endl;
    internals::hal::gpio::setFakeADCReading({0, 1000, 2000, 3000, 4000, 4095, 4000, 3000, 2000, 1000});

    BatteryStatus status = waitFor();
    assert<bool>::equals("known charging state", true, status.isCharging.has_value());
    assert<bool>::equals("charging state", true, status.isCharging.value());
    assert<bool>::equals("known wired power state", true, status.usingExternalPower.has_value());
    assert<bool>::equals("wired power state", true, status.usingExternalPower.value());
    assert<bool>::equals("known battery presence", false, status.isBatteryPresent.has_value());
    assert<bool>::equals("known SoC state", false, status.stateOfCharge.has_value());
}

void test3()
{
    std::cout << "- test 3 (no battery) -" << std::endl;
    internals::hal::gpio::setFakeADCReading({20});

    BatteryStatus status = waitFor();
    BatteryService::call::getStatus(status);
    assert<bool>::equals("known charging state", true, status.isCharging.has_value());
    assert<bool>::equals("charging state", false, status.isCharging.value());
    assert<bool>::equals("known wired power state", true, status.usingExternalPower.has_value());
    assert<bool>::equals("wired power state", true, status.usingExternalPower.value());
    assert<bool>::equals("known SoC state", false, status.stateOfCharge.has_value());
    assert<bool>::equals("known battery presence", true, status.isBatteryPresent.has_value());
    assert<bool>::equals("battery presence", false, status.isBatteryPresent.value());
}

void test4()
{
    std::cout << "- test 4 (constant voltage charging simulation) -" << std::endl;
    internals::hal::gpio::setFakeADCReading({3500, 3560, 3540, 3520});

    BatteryStatus status = waitFor();
    BatteryService::call::getStatus(status);
    assert<bool>::equals("known charging state", true, status.isCharging.has_value());
    assert<bool>::equals("charging state", true, status.isCharging.value());
    assert<bool>::equals("known wired power state", true, status.usingExternalPower.has_value());
    assert<bool>::equals("wired power state", true, status.usingExternalPower.value());
    assert<bool>::equals("known SoC state", false, status.stateOfCharge.has_value());
    assert<bool>::equals("known battery presence", false, status.isBatteryPresent.has_value());
}

void test5()
{
    std::cout << "- test 5 (discharging simulation) -" << std::endl;
    internals::hal::gpio::setFakeADCReading({2000, 2020, 1990, 2005});

    BatteryStatus status = waitFor();
    BatteryService::call::getStatus(status);
    assert<bool>::equals("known charging state", true, status.isCharging.has_value());
    assert<bool>::equals("charging state", false, status.isCharging.value());
    assert<bool>::equals("known wired power state", false, status.usingExternalPower.has_value());
    assert<bool>::equals("known SoC state", true, status.stateOfCharge.has_value());
    assert<bool>::equals("known battery presence", true, status.isBatteryPresent.has_value());
    assert<bool>::equals("battery presence", true, status.isBatteryPresent.value());
    assert<int>::more("SoC>18", 18, status.stateOfCharge.value());
    assert<int>::less("SoC<21", 21, status.stateOfCharge.value());
}

//------------------------------------------------------------------
//------------------------------------------------------------------
// Entry point
//------------------------------------------------------------------
//------------------------------------------------------------------

int main()
{
    BatteryCalibrationService::inject(new BatteryCalibrationMock());
    OnBatteryStatus.subscribe(get_current_battery_level);
    batteryMonitor::configure(TEST_RTC_GPIO1);
    hardware = static_cast<VoltageDividerMonitor *>(internals::batteryMonitor::getHardwareInstance());
    assert(hardware != nullptr);

    batteryMonitor::setPeriod(1);
    internals::batteryMonitor::getReady();
    internals::hal::gpio::setFakeADCReading({2000});
    OnStart();
    running = true;

    test1();
    test2();
    test3();
    test4();
    test5();

    return 0;
}
