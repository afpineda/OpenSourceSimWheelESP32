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

#include "SimWheelTypes.h"
#include "debugUtils.h"
#include "ShiftRegistersInput.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

ShiftRegistersInput *btns;
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

    btns = new ShiftRegistersInput(
        TEST_SR_SERIAL,
        TEST_SR_LOAD,
        TEST_SR_NEXT,
        srNumbers,
        sizeof(srNumbers)/sizeof(srNumbers[0])
      );

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