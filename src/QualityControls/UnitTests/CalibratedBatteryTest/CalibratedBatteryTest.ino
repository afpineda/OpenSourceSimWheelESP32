/**
 * @file CalibratedBatteryTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-08-27
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <HardwareSerial.h>
#include "SimWheel.h"
#include "debugUtils.h"

//-------------------------------------------------------
// Globals
//-------------------------------------------------------

int lastLevel = -100;

//-------------------------------------------------------
// Auxiliary
//-------------------------------------------------------

bool hasCalibrationData()
{
    int calData = 0;
    int sum = 0;
    for (uint8_t index = 0; (index <= 255) && (calData >= 0); index++)
    {
        calData = batteryCalibration::getCalibration(index);
        if (calData > 0)
            sum += calData;
    }
    return (sum > 0);
}

//-------------------------------------------------------
// Mocks
//-------------------------------------------------------

void hidImplementation::reportBatteryLevel(int level)
{
    if (level != lastLevel)
    {
        lastLevel = level;
        Serial.printf("SoC: %d\n", lastLevel);
    }
}

void notify::lowBattery()
{
}

void notify::powerOff()
{
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
    batteryMonitor::configureForTesting();
    batteryMonitor::begin(
        TEST_BATTERY_READ_ENABLE,
        TEST_BATTERY_READ);
    if (!hasCalibrationData())
    {
        Serial.println("WARNING: no calibration data found.");
        Serial.println("Using auto-calibration algorithm.");
    }
    Serial.println("--GO--");
}

void loop()
{
    vTaskDelay(portMAX_DELAY);
}
