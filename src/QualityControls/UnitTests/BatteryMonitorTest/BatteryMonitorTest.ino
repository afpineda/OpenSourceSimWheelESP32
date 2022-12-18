/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-04-17
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
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

int gtl = 0;

int batteryCalibration::getBatteryLevel(int reading)
{
    gtl = reading;
    return -1;
}

int batteryCalibration::getBatteryLevelAutoCalibrated(int reading)
{
    serialPrintf("Reading: %d\n", reading);
    if (gtl!=reading) {
        Serial.println("ERROR: ADC reading at getBatteryLevelAutoCalibrated() differs from reading at getBatteryLevel()");
    }
    return 66;
}

void hidImplementation::reportBatteryLevel(int level)
{

}

void ui::showLowBatteryNotice()
{
}

void ui::turnOff()
{
}

void batteryCalibration::begin()
{
}

void capabilities::setFlag(deviceCapability_t a, bool b)
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
    int testnumber = 1;

    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("--GO--");
    power::startBatteryMonitor(
        TEST_BATTERY_READ_ENABLE,
        TEST_BATTERY_READ,
        true);
}

void loop()
{
    delay(60 * 1000);
}
