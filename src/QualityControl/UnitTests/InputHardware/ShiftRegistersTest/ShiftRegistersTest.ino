/**
 * @file ShiftRegistersTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-19
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

ShiftRegistersInput *buttons;
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
    ShiftRegisterChain spec;
    InputNumber SER;
    setDebugInputNumbers(spec, SER);

    buttons = new ShiftRegistersInput(
        TEST_SR_LOAD,
        TEST_SR_NEXT,
        TEST_SR_SERIAL,
        spec,
        SER);

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