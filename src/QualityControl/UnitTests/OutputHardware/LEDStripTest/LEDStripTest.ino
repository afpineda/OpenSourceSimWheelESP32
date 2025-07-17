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
    Serial.println("Go red");
    strip->pixelRangeRGB(0, LED_COUNT - 1, 255, 0, 0);
    strip->show();
    DELAY_MS(DEFAULT_DELAY);
    Serial.println("Go green");
    strip->pixelRangeRGB(0, LED_COUNT - 1, 0, 255, 0);
    strip->show();
    DELAY_MS(DEFAULT_DELAY);
    Serial.println("Go blue");
    strip->pixelRangeRGB(0, LED_COUNT - 1, 0, 0, 255);
    strip->show();
    DELAY_MS(DEFAULT_DELAY);
    Serial.println("Go white");
    strip->pixelRangeRGB(0, LED_COUNT - 1, 255, 255, 255);
    strip->show();
    DELAY_MS(DEFAULT_DELAY);
    Serial.println("Go purple");
    strip->pixelRangeRGB(0, LED_COUNT - 1, 128, 0, 128);
    strip->show();
    DELAY_MS(DEFAULT_DELAY);
    Serial.println("Go orange");
    strip->pixelRangeRGB(0, LED_COUNT - 1, 255, 65, 0);
    strip->show();
    DELAY_MS(DEFAULT_DELAY);
    Serial.println("Go orange dimmer");
    strip->pixelRangeRGB(0, LED_COUNT - 1, 64, 16, 0);
    strip->show();
    DELAY_MS(DEFAULT_DELAY);
    Serial.println("Go off");
    strip->clear();
    strip->show();
    DELAY_MS(DEFAULT_DELAY);
    Serial.println("rainbow");
    strip->pixelRGB(0, 0xEE82EE);
    strip->pixelRGB(1, 0x4B0082);
    strip->pixelRGB(2, 0x0000FF);
    strip->pixelRGB(3, 0x008000);
    strip->pixelRGB(4, 0xFFFF00);
    strip->pixelRGB(5, 0xFFA500);
    strip->pixelRGB(6, 0xFF0000);
    strip->show();
    DELAY_MS(DEFAULT_DELAY);
    Serial.println("Shift to next");
    strip->shiftToNext();
    strip->show();
    DELAY_MS(DEFAULT_DELAY);
    Serial.println("Shift to previous");
    strip->shiftToPrevious();
    strip->show();
    DELAY_MS(DEFAULT_DELAY);
}