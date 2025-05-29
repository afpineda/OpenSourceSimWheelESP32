/**
 * @file RotaryCodedSwitchTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-05-29
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

RotaryCodedSwitchInput *buttons;
uint64_t state = 0ULL;

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    Serial.println("-- READY --");

    RotaryCodedSwitch sw;
    for (int i = 0; i < 8; i++)
        sw[i] = 10 + i;

    buttons = new RotaryCodedSwitchInput(
        sw,
        {TEST_AMTXER_SEL1,
         TEST_AMTXER_SEL2,
         TEST_AMTXER_SEL3},
        true);

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