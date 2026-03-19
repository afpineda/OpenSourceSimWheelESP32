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
uint128_t oldState{};

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
    std::vector<OutputGPIO> selectors = getDebugMuxSelectors();

    buttons = new AnalogMultiplexerInput(
        selectors[0],
        selectors[1],
        selectors[2],
        spec);

    Serial.println("-- GO --");
}

void loop()
{
    uint128_t newState{};
    buttons->read(newState);
    if (oldState != newState)
    {
        oldState = newState;
        debugPrintBool(newState.low);
        Serial.println("");
    }
    DELAY_MS(60);
}