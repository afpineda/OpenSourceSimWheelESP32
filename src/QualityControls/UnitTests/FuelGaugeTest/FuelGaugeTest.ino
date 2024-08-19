/**
 * @file FuelGaugeTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-08-19
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <Arduino.h>
#include "SimWheel.h"
#include "debugUtils.h"

//-------------------------------------------------------
// Globals
//-------------------------------------------------------

//-------------------------------------------------------
// Mocks
//-------------------------------------------------------

int currentSoC = -100;

void hidImplementation::reportBatteryLevel(int level)
{
    if (currentSoC != level)
    {
        currentSoC = level;
        Serial.printf("SoC: %d\n", level);
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
    batteryMonitor::begin();
    Serial.println("--GO--");
}

void loop()
{
    delay(60 * 1000);
}
