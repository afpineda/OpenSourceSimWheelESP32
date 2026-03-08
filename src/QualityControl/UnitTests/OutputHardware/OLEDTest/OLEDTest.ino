/**
 * @file OLEDTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2026-03-02
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "OutputHardware.hpp"
#include "Testing.hpp"
// #include "esp32-hal.h"
#include "HAL.hpp" // For DELAY_MS()
#include "Adafruit_GFX.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

OLED hw;

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

GFXcanvas1 test_frame()
{
    GFXcanvas1 result(128, 64);
    result.drawRoundRect(0, 0, 128, 64, 3, 0xFF);
    result.drawLine(0, 0, 127, 63, 0xFF);
    result.drawLine(127, 0, 0, 63, 0xFF);
    result.drawEllipse(64, 32, 64, 32, 0xFF);
    return result;
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    debugPrintBegin();
    debugPrintf("SCL: %u, SDA: %u\n", SCL, SDA);
    hw = OLED(
        OLEDParameters::for132x64(),
        I2CBus::PRIMARY);

    if (hw.available())
    {
        debugPrintf("--GO--\n");
        hw.clear();
        hw.show(test_frame().getBuffer());
    }
    else
        debugPrintf("Device not found. Test cancelled\n");
}

void loop()
{
    DELAY_MS(3000);
    hw.inverse_display(true);
    DELAY_MS(3000);
    hw.inverse_display(false);
}