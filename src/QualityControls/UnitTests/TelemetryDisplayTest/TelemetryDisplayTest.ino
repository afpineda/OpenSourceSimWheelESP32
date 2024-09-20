/**
 * @file TelemetryDisplayTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-09-20
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

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    Serial.println("-- READY --");
    notify::begin({new SerialTelemetryDisplay()}, 1);
}

void loop()
{
    notify::telemetryData.frameID++;
    delay(1100);
    notify::telemetryData.frameID++;
    delay(3000);
}