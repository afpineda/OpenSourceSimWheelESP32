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

#include "Testing.hpp"
#include "InputHardware.hpp"
#include "HAL.hpp"

#include <HardwareSerial.h>

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

uint64_t globalState = 0;
uint64_t mask;
RotaryEncoderInput *rot1 = nullptr;
RotaryEncoderInput *rot2 = nullptr;

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

void notifyInputEvent(uint64_t state)
{
    uint64_t changes = globalState ^ state;
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

    rot1 = new RotaryEncoderInput(TEST_ROTARY_CLK, TEST_ROTARY_DT, 5, 6, false);
    rot2 = new RotaryEncoderInput(TEST_ROTARY_ALPS_A, TEST_ROTARY_ALPS_B, 0, 1, true);

    mask = rot1->mask & rot2->mask;
    Serial.println("-- GO --");
}

void loop()
{
    uint64_t r1 = rot1->read(globalState);
    uint64_t r2 = rot2->read(globalState);
    uint64_t newState = (r1 | r2);
    if (globalState != newState)
    {
        notifyInputEvent(newState);
        globalState = newState;
    }
    else if (newState != 0)
    {
        Serial.println("Pulse delay");
    }
    DELAY_MS(60);
    if (Serial.available())
    {
        int key = Serial.read();
        if ((key >= '1') && (key <= '6'))
        {
            RotaryEncoderInput::setPulseMultiplier(key - '1' + 1);
            Serial.printf(
                "Pulse multiplier set to %hhu\n",
                RotaryEncoderInput::pulseMultiplier);
        }
    }
}