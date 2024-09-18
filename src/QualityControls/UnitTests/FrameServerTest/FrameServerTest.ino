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
#include <esp_task_wdt.h>

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

class TestImpl : public AbstractNotificationInterface
{
public:
    virtual void onStart() override
    {
        Serial.println("begin");
    };

    virtual void onBitePoint() override
    {
        Serial.println("bite point");
    };

    virtual void onConnected() override
    {
        Serial.println("connected");
    };

    virtual void onBLEdiscovering() override
    {
        Serial.println("BLE discovering");
    };

    virtual void onLowBattery() override
    {
        Serial.println("Low battery");
    };

    virtual void serveSingleFrame() override
    {
        Serial.println("(FRAME)");
    };
};

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

    notify::begin({new TestImpl()}, 3);
}

void loop()
{
    delay(1000);
    notify::connected();
    delay(2000);
    notify::bitePoint();
}