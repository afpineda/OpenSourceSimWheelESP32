/**
 * @file RotaryEncoderTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Input from rotary encoders
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <Arduino.h>
#include "RotaryEncoderInput.h"
#include "debugUtils.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

inputBitmap_t globalState = 0;
inputBitmap_t mask;
RotaryEncoderInput *rotaries = nullptr;

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

void notifyInputEvent(inputBitmap_t mask, inputBitmap_t state)
{
    inputBitmap_t changes = globalState ^ state;
    globalState = state;
    Serial.print("MASK  : ");
    debugPrintBool(mask);
    Serial.println("");
    Serial.print("STATE : ");
    debugPrintBool(state);
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
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    Serial.begin(115200);
    Serial.println("-- READY --");

    rotaries = new RotaryEncoderInput(TEST_ROTARY_CLK, TEST_ROTARY_DT, 5, 6, false, rotaries);
    rotaries = new RotaryEncoderInput(TEST_ROTARY_ALPS_A, TEST_ROTARY_ALPS_B, 0, 1, true, rotaries);

    mask = RotaryEncoderInput::getChainMask(rotaries);
    Serial.println("-- GO --");
}

void loop()
{
    inputBitmap_t newState = RotaryEncoderInput::readInChain(globalState, rotaries);
    if (globalState != newState)
    {
        notifyInputEvent(mask, newState);
        globalState = newState;
    }
    else if (newState != 0)
    {
        Serial.println("Pulse delay");
    }
    vTaskDelay(DEBOUNCE_TICKS);
    if (Serial.available())
    {
        int key = Serial.read();
        if (key == '3')
        {
            RotaryEncoderInput::setPulseMultiplier(3);
            Serial.println("Pulse multiplier set to 3");
        }
        else if (key == '2')
        {
            RotaryEncoderInput::setPulseMultiplier(2);
            Serial.println("Pulse multiplier set to 2");
        }
        else if (key == '1')
        {
            RotaryEncoderInput::setPulseMultiplier(1);
            Serial.println("Pulse multiplier set to 1");
        }
    }
}