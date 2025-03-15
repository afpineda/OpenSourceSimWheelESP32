/**
 * @file BatteryMonitorTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-04-17
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "Testing.hpp"
#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include "HAL.hpp"

#include <HardwareSerial.h>

//-------------------------------------------------------
// Globals
//-------------------------------------------------------

int currentSoC = -100;

//-------------------------------------------------------
// Mocks
//-------------------------------------------------------

class BatteryCalibrationMock : public BatteryCalibrationService
{
public:
    virtual int getBatteryLevel(int reading) override
    {
        return reading / 41; // 0-4096 <=> 0-99 linear
    }
} calMock;

void reportBatteryLevel(int level)
{
    if (currentSoC != level)
    {
        currentSoC = level;
        Serial.printf("SoC: %d\n", level);
    }
}

//-------------------------------------------------------
// Auxiliary
//-------------------------------------------------------

//-------------------------------------------------------
// Entry point
//-------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    Serial.println("--READY--");
    OnBatteryLevel::subscribe(reportBatteryLevel);
    BatteryCalibrationService::inject(&calMock);
    internals::batteryMonitor::configureForTesting();
    batteryMonitor::configure(
        TEST_BATTERY_READ,
        TEST_BATTERY_READ_ENABLE);
    internals::batteryMonitor::getReady();
    OnStart::notify();
    Serial.println("--GO--");
}

void loop()
{
    DELAY_MS(60 * 1000);
}
