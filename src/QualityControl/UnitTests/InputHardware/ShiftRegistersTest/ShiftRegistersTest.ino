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
uint128_t state{};

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

    Serial.println("-- GO --");
}

void loop()
{
    uint128_t oldState = state;
    buttons->read(state);
    if (oldState != state)
    {
        debugPrintBool(state.low);
        Serial.println("");
    }
    DELAY_MS(60);
}