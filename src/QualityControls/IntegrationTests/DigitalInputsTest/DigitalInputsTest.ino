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

#define ROTARY_PUSH_BN 10
#define ROTARY_CW_BN 12
#define ROTARY_CCW_BN 13

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
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("-- READY --");
    inputs::begin();
    inputs::addButtonMatrix(
        mtxSelectors,
        sizeof(mtxSelectors) / sizeof(mtxSelectors[0]),
        mtxInputs,
        sizeof(mtxInputs) / sizeof(mtxInputs[0]),
        mtxNumbers);
    inputs::addDigital(TEST_ROTARY_SW, ROTARY_PUSH_BN, true, true);
    inputs::addRotaryEncoder(TEST_ROTARY_CLK, TEST_ROTARY_DT, ROTARY_CW_BN, ROTARY_CCW_BN,false);

    Serial.println("-- GO --");
    inputs::start();
}

void loop()
{
    delay(5000);
}