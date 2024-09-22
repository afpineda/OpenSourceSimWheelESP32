/**
 * @file FrameServerTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-08
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <Arduino.h>
#include "SimWheel.h"
#include "SerialNotification.h"
#include <esp_task_wdt.h>

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

volatile clutchValue_t userSettings::bitePoint = (clutchValue_t)0;
volatile uint8_t userSettings::uiPage[MAX_UI_COUNT];

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    Serial.println("-- READY --");
    notify::begin({new SerialNotificationImpl()}, 3);
}

void loop()
{
    delay(1000);
    notify::connected();
    delay(2000);
    notify::bitePoint();
}