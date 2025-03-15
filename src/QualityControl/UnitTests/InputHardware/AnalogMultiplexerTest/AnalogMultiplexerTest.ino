/**
 * @file AnalogMultiplexerTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-03
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "Testing.hpp"
#include "InputHardware.hpp"
#include "HAL.hpp"

#include <HardwareSerial.h>

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

AnalogMultiplexerInput *buttons;
uint64_t state = 0;

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    Serial.println("-- READY --");

    AnalogMultiplexerGroup<Mux8Pin> spec;
    setDebugInputNumbers(spec);
    OutputGPIOCollection selectors = getDebugMuxSelectors();

    buttons = new AnalogMultiplexerInput(
        selectors[0],
        selectors[1],
        selectors[2],
        spec);

    Serial.println("MASK:");
    debugPrintBool(buttons->mask);
    Serial.println("");
    Serial.println("-- GO --");
}

void loop()
{
    uint64_t newState = buttons->read(state);
    if (state != newState)
    {
        state = newState;
        debugPrintBool(state);
        Serial.println("");
    }
    DELAY_MS(60);
}