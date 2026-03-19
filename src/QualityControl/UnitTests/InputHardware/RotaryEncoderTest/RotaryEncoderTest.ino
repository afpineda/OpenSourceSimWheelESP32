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

uint128_t globalState{};
RotaryEncoderInput *rot1 = nullptr;
RotaryEncoderInput *rot2 = nullptr;

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

void notifyInputEvent(const uint128_t &state)
{
    Serial.print("STATE : ");
    debugPrintBool(state.low);
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

    rot1 = new RotaryEncoderInput(
        TEST_ROTARY_CLK,
        TEST_ROTARY_DT, 0, 1, false);
    rot2 = new RotaryEncoderInput(
        TEST_ROTARY_ALPS_A,
        TEST_ROTARY_ALPS_B, 5, 6, true);

    Serial.println("-- GO --");
}

void loop()
{
    uint128_t newState{globalState};
    rot1->read(newState);
    rot2->read(newState);
    if (globalState != newState)
    {
        notifyInputEvent(newState);
        globalState = newState;
    }
    else if (newState)
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