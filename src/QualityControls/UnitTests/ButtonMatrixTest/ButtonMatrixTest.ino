/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-03
 * @brief Unit Test. See [README](./README.md)
 * 
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 * 
 */

#include <Arduino.h>
#include "SimWheel.h"
#include "debugUtils.h"
#include "ButtonMatrixInput.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

ButtonMatrixInput *btns;
inputBitmap_t state = 0;

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("-- READY --");
    btns = new ButtonMatrixInput(3);
    btns->addInputPin(TEST_BTNMTX_COL1);
    btns->addInputPin(TEST_BTNMTX_COL2);
    btns->addSelectorPin(TEST_BTNMTX_ROW1);
    btns->addSelectorPin(TEST_BTNMTX_ROW2);
    btns->addSelectorPin(TEST_BTNMTX_ROW3);
    Serial.println("MASK:");
    debugPrintBool(btns->mask);
    Serial.println("");
    Serial.println("-- GO --");
}

void loop()
{
    inputBitmap_t newState = btns->read(state);
    if (state != newState)
    {
        state = newState;
        debugPrintBool(state);
        Serial.println("");
    }
//    delay(100);
    vTaskDelay(DEBOUNCE_TICKS);
}