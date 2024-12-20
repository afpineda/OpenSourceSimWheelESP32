/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-04-18
 * @brief Integration test. See [Readme](./README.md)
 *
 * @copyright Licensed under the EUPL
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

void notify::shutdown()
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
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("-- READY --");
    batteryCalibration::clear();
    batteryMonitor::configureForTesting();
    batteryMonitor::begin(TEST_BATTERY_READ_ENABLE, TEST_BATTERY_READ);
    Serial.println("-- GO --");
}

void loop()
{
    delay(10000);
}