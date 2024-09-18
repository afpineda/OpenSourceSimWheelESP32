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

volatile clutchValue_t userSettings::bitePoint = (clutchValue_t)0;

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);

    notify::begin({new SerialNotificationImpl()});
    notify::BLEdiscovering();
    notify::connected();
    notify::lowBattery();
    for (clutchValue_t i = CLUTCH_NONE_VALUE; i < CLUTCH_FULL_VALUE; i++)
    {
        userSettings::bitePoint = i;
        notify::bitePoint();
    }

    delay(8000);
    Serial.println("-- END --");
}

void loop()
{
    delay(2000);
}