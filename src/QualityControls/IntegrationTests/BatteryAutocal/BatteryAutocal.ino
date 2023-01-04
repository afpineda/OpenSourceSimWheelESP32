/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-04-18
 * @brief Integration test. See [Readme](./README.md)
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "debugUtils.h"
#include "SimWheel.h"

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

void hidImplementation::reportBatteryLevel(int l)
{
    Serial.print("Last Battery level: ");
    Serial.print(l);
    Serial.println("");
}

void notify::lowBattery()
{

}

void notify::powerOff()
{

}

void capabilities::setFlag(deviceCapability_t a, bool b)
{

}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("-- READY --");
    batteryCalibration::clear();
    power::startBatteryMonitor(TEST_BATTERY_READ_ENABLE, TEST_BATTERY_READ, true);
    Serial.println("-- GO --");
}

void loop()
{
  delay(10000);
}