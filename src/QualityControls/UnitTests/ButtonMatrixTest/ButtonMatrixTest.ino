/**
 * @file ButtonMatrixTest.ino
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
#include "ButtonMatrixInput.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

ButtonMatrixInput *buttons;
inputBitmap_t state = 0ULL;

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

    buttons = new ButtonMatrixInput(
        mtxSelectors,
        mtxInputs);
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
    //    delay(100);
    vTaskDelay(DEBOUNCE_TICKS);
}