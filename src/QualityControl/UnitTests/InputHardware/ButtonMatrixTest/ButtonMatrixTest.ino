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
uint64_t state = 0ULL;

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