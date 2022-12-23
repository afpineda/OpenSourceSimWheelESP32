/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-01
 * @brief Integration test. See [Readme](./README.md)
 * 
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 * 
 */

#include <Arduino.h>
#include "debugUtils.h"
#include "SimWheel.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------


//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

void inputHub::onStateChanged(inputBitmap_t globalState, inputBitmap_t changes)
{
    Serial.print("STATE : ");
    debugPrintBool(globalState);
    Serial.println("");
    Serial.print("CHANGE: ");
    debugPrintBool(changes);
    Serial.println("");
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("-- READY --");
    inputs::begin();
    inputs::addButtonMatrix(mtxSelectors,3,mtxInputs,2,mtxNumbers);
    inputs::addDigital(TEST_ROTARY_SW,true,true,6);
    inputs::addRotaryEncoder(TEST_ROTARY_CLK,TEST_ROTARY_DT,7,8);

    Serial.println("-- GO --");
    inputs::start();
}

void loop()
{
    delay(5000);
}