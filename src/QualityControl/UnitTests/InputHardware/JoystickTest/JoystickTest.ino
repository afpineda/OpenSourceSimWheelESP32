/**
 * @file JoystickTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2026-04-17
 * @brief Input from analog joysticks
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "Testing.hpp"
#include "InputHardware.hpp"
#include "HAL.hpp"

#include "HardwareSerial.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

AnalogJoystickInput *input;
uint128_t oldState{};

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    Serial.println("-- READY --");

    input = new AnalogJoystickInput(
        TEST_ANALOG_PIN1,
        TEST_ANALOG_PIN2,
        0, 1, 2, 3);
    Serial.println("-- GO --");
}

void loop()
{
    uint128_t newState{};
    input->read(newState);
    if (oldState != newState)
    {
        oldState = newState;
        debugPrintBool(newState.low);
        Serial.println("");
    }
    DELAY_MS(60);
}