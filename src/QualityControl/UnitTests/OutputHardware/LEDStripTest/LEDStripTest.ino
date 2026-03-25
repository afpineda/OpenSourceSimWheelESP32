/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-10-24
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "Testing.hpp"
#include "OutputHardware.hpp"
#include "HAL.hpp"

#include <HardwareSerial.h>

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

#define LED_COUNT 8
#define DEFAULT_DELAY 2000

LEDStrip *strip;

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    strip = new LEDStrip(TEST_D_OUT, LED_COUNT, TEST_LEVEL_SHIFTER);
    strip->brightness(128);
    Serial.println("--GO--");
}

void loop()
{
    {
        Serial.println("Go red");
        auto v = strip->pixelVector(0xFF0000);
        strip->show(v);
        DELAY_MS(DEFAULT_DELAY);
    }
    {
        Serial.println("Go green");
        auto v = strip->pixelVector(0x00FF00);
        strip->show(v);
        DELAY_MS(DEFAULT_DELAY);
    }
    {
        Serial.println("Go blue");
        auto v = strip->pixelVector(0x0000FF);
        strip->show(v);
        DELAY_MS(DEFAULT_DELAY);
    }
    {
        Serial.println("Go white");
        auto v = strip->pixelVector(0xFFFFFF);
        strip->show(v);
        DELAY_MS(DEFAULT_DELAY);
    }
    {
        Serial.println("Go purple");
        auto v = strip->pixelVector(0x800080);
        strip->show(v);
        DELAY_MS(DEFAULT_DELAY);
    }
    {
        Serial.println("Go orange");
        auto v = strip->pixelVector(0xFF4100);
        strip->show(v);
        DELAY_MS(DEFAULT_DELAY);
    }
    {
        Serial.println("Go orange dimmer");
        auto v = strip->pixelVector(0x411000);
        strip->show(v);
        DELAY_MS(DEFAULT_DELAY);
    }
    {
        Serial.println("Go off");
        strip->clear();
        DELAY_MS(DEFAULT_DELAY);
    }
    {
        Serial.println("rainbow");
        auto v = strip->pixelVector();
        v[0] = 0xEE82EE;
        v[1] = 0x4B0082;
        v[2] = 0x0000FF;
        v[3] = 0x008000;
        v[4] = 0xFFFF00;
        v[5] = 0xFFA500;
        v[6] = 0xFF0000;
        strip->show(v);
        DELAY_MS(DEFAULT_DELAY);

        Serial.println("Shift to next");
        PixelVectorHelper::shift_right(v, 1);
        strip->show(v);
        DELAY_MS(DEFAULT_DELAY);

        Serial.println("Shift to previous");
        PixelVectorHelper::shift_left(v, 1);
        strip->show(v);
        DELAY_MS(DEFAULT_DELAY);
    }
}