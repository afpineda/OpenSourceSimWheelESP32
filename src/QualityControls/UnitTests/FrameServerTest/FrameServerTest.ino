/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-08
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "SimWheel.h"
#include <esp_task_wdt.h>

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

class TestImpl: public AbstractFrameServerInterface
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

    virtual void powerOn() {
        Serial.println("powerOn");
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
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("-- READY --");

    notify::begin(new TestImpl());

    Serial.println("-- GO --");
}

void loop()
{
    delay(1000);
    notify::powerOn();
    delay(2000);
    notify::powerOff();
}