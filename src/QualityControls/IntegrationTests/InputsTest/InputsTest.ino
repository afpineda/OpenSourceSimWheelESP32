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
    inputNumber_t n;
    
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("-- READY --");
    inputs::begin();

    n = inputs::setButtonMatrix(mtxSelectors,3,mtxInputs,2);
    // n = 0
    Serial.println(n);
    n = inputs::addDigital(TEST_ROTARY_SW,true,false);
    // n = 6
    Serial.println(n);
    n = inputs::addRotaryEncoder(TEST_ROTARY_CLK,TEST_ROTARY_DT);
    //n = inputs::addRotaryEncoder(TEST_ROTARY_ALT_A,TEST_ROTARY_ALT_B,true);
    // n = 7
    Serial.println(n);

    Serial.println("-- GO --");
    inputs::start();
}

void loop()
{
    delay(5000);
}