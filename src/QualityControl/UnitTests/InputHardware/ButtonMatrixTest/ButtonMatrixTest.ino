/**
 * @file ButtonMatrixTest.ino
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

ButtonMatrixInput *buttons;
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
    ButtonMatrix spec;
    setDebugInputNumbers(spec);

    buttons = new ButtonMatrixInput(spec, false);

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