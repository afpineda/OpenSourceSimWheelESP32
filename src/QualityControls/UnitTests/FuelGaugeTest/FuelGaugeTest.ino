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

#include <HardwareSerial.h>
#include "SimWheel.h"
#include "debugUtils.h"

//-------------------------------------------------------
// Globals
//-------------------------------------------------------

uint8_t compensation = 0;

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

//-------------------------------------------------------
// Auxiliary
//-------------------------------------------------------

// void printCompensation()
// {
//     if (max1704x_getCompensation(compensation))
//         Serial.printf("ModelGauge compensation: %d (dec).\n", compensation);
//     else
//         Serial.printf("ModelGauge compensation not available.\n");
// }

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
    // if (Serial.available())
    // {
    //     char ch = Serial.read();
    //     if (ch = 'c')
    //         printCompensation();
    //     else if ((ch = '+') && (compensation < 255))
    //     {
    //         compensation++;
    //         if (max1704x_setCompensation(compensation))
    //             printCompensation();
    //     }
    //     else if ((ch = '-') && (compensation > 0))
    //     {
    //         compensation--;
    //         if (max1704x_setCompensation(compensation))
    //             printCompensation();
    //     }
    // }
    delay(250);
}
