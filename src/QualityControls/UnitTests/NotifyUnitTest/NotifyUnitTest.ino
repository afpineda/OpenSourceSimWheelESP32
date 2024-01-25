/**
 * @file NotifyUnitTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <Arduino.h>
#include "SimWheel.h"
#include "SimWheelTypes.h"
#include "SerialNotification.h"
// #include "debugUtils.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    Serial.println("-- READY --");

    notify::begin(new SerialNotificationImpl());

    Serial.println("-- GO --");

    notify::BLEdiscovering();
    notify::connected();
    notify::lowBattery();
    notify::powerOff();

    for (clutchValue_t i = CLUTCH_NONE_VALUE; i < CLUTCH_FULL_VALUE; i++)
        notify::bitePoint(i);

    delay(8000);
    Serial.println("-- END --");
}

void loop()
{
    delay(2000);
}