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

class TestImpl: public AbstractNotificationInterface
{
public:
    virtual void begin() {
        Serial.println("begin");
    };

    virtual void bitePoint(clutchValue_t bitePoint) {
        Serial.println("bite point");
    };

    virtual void connected() {
        Serial.println("connected");
    };

    virtual void BLEdiscovering() {
        Serial.println("BLE discovering");
    };

    virtual void powerOff() {
        Serial.println("powerOff");
    }

    virtual void lowBattery() {
        Serial.println("Low battery");
    }

    virtual void serveSingleFrame() {
        Serial.println("(FRAME)");
    };

    virtual uint8_t getTargetFPS() {
        return 3;
    }
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

    notify::begin(new TestImpl());

    Serial.println("-- GO --");
}

void loop()
{
    delay(1000);
    notify::connected();
    delay(2000);
    notify::powerOff();
}