/**
 * @file AnalogMultiplexerTest.ino
 *
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
#include "AnalogMultiplexerInput.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

AnalogMultiplexerInput *btns;
inputBitmap_t state = 0;

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    Serial.println("-- READY --");

    btns = new AnalogMultiplexerInput(
        amtxerSelectors,
        sizeof(amtxerSelectors) / sizeof(amtxerSelectors[0]),
        amtxerInputs,
        sizeof(amtxerInputs) / sizeof(amtxerInputs[0]),
        amtxerNumbers);

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
    vTaskDelay(DEBOUNCE_TICKS);
}