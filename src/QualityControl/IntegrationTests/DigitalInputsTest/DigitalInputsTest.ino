/**
 * @file DigitalInputsTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-01
 * @brief Integration test. See [Readme](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "Testing.hpp"
#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"

#include <HardwareSerial.h>

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

#define ROTARY_PUSH_BN 10
#define ROTARY_CW_BN 12
#define ROTARY_CCW_BN 13

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

void internals::inputHub::onRawInput(DecouplingEvent &input)
{
    Serial.print("STATE : ");
    debugPrintBool(input.rawInputBitmap);
    Serial.println("");
    Serial.print("CHANGE: ");
    debugPrintBool(input.rawInputChanges);
    Serial.println("");
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    Serial.println("-- READY --");

    ButtonMatrix mtx;
    setDebugInputNumbers(mtx);

    inputs::addButtonMatrix(mtx);

    inputs::addButton(TEST_ROTARY_SW, ROTARY_PUSH_BN);
    inputs::addRotaryEncoder(
        TEST_ROTARY_CLK,
        TEST_ROTARY_DT,
        ROTARY_CW_BN,
        ROTARY_CCW_BN,
        false);

    internals::inputs::getReady();
    OnStart::notify();
    Serial.println("-- GO --");
}

void loop()
{
    delay(5000);
}