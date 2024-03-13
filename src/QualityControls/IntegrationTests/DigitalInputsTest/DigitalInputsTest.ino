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

#include <HardwareSerial.h>
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

void inputHub::onRawInput(
    inputBitmap_t rawInputBitmap,
    inputBitmap_t rawInputChanges,
    clutchValue_t leftAxis,
    clutchValue_t rightAxis,
    bool axesChanged)
{
    Serial.print("STATE : ");
    debugPrintBool(rawInputBitmap);
    Serial.println("");
    Serial.print("CHANGE: ");
    debugPrintBool(rawInputChanges);
    Serial.println("");
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    Serial.println("-- READY --");
    auto &matrix = inputs::addButtonMatrix(mtxSelectors, mtxInputs);
    setDebugInputNumbers(matrix);
    auto &mux = inputs::addAnalogMultiplexer(amtxerSelectors, amtxerInputs);
    setDebugInputNumbers(mux);

    inputs::addDigital(TEST_ROTARY_SW, ROTARY_PUSH_BN);
    inputs::addRotaryEncoder(TEST_ROTARY_CLK, TEST_ROTARY_DT, ROTARY_CW_BN, ROTARY_CCW_BN, false);

    inputs::start();
    Serial.println("-- GO --");
}

void loop()
{
    delay(5000);
}