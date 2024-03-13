/**
 * @file AnalogMultiplexerTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-03
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <HardwareSerial.h>
#include "SimWheelTypes.h"
#include "debugUtils.h"
#include "AnalogMultiplexerInput.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

AnalogMultiplexerInput *buttons;
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

    buttons = new AnalogMultiplexerInput(
        amtxerSelectors,
        amtxerInputs);
    setDebugInputNumbers(*buttons);

    Serial.println("MASK:");
    debugPrintBool(buttons->mask);
    Serial.println("");
    Serial.println("-- GO --");
}

void loop()
{
    inputBitmap_t newState = buttons->read(state);
    if (state != newState)
    {
        state = newState;
        debugPrintBool(state);
        Serial.println("");
    }
    vTaskDelay(DEBOUNCE_TICKS);
}