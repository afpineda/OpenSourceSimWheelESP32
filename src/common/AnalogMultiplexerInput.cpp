/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-01-19
 * @brief Use of analog multiplexors as inputs
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include "AnalogMultiplexerInput.h"
// #include <Arduino.h>

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

#define SIGNAL_CHANGE_DELAY_TICKS 1

// ----------------------------------------------------------------------------
// Constructors
// ----------------------------------------------------------------------------

AnalogMultiplexerInput::AnalogMultiplexerInput(
    const gpio_num_t selectorPins[],
    const uint8_t selectorPinCount,
    const gpio_num_t inputPins[],
    const uint8_t inputPinCount,
    inputNumber_t *buttonNumbersArray,
    const bool negativeLogic,
    DigitalPolledInput *nextInChain) : DigitalPolledInput(nextInChain)
{
    this->switchCount = (1 << selectorPinCount) * inputPinCount;
    this->selectorPinCount = selectorPinCount;
    this->inputPinCount = inputPinCount;
    this->selectorPins = selectorPins;
    this->inputPins = inputPins;
    this->buttonNumbersArray = buttonNumbersArray;
    this->negativeLogic = negativeLogic;

    if ((switchCount > 64) || (switchCount == 0))
    {
        log_e("Too few/many input or selector pins at AnalogMultiplexerInput::AnalogMultiplexerInput()");
        abort();
    }

    if (buttonNumbersArray != nullptr)
        updateMask(buttonNumbersArray, switchCount);
    else
    {
        log_e("Unknown input numbers in call to AnalogMultiplexerInput::AnalogMultiplexerInput()");
        abort();
    }

    // Initialize debouncing state
    this->debounce = 0;

    // Check and initialize pins
    for (int i = 0; i < selectorPinCount; i++)
        checkAndInitializeSelectorPin(selectorPins[i]);
    for (int i = 0; i < inputPinCount; i++)
        checkAndInitializeInputPin(inputPins[i], !negativeLogic, negativeLogic);
}

// ----------------------------------------------------------------------------
// Polling
// ----------------------------------------------------------------------------

inputBitmap_t AnalogMultiplexerInput::read(inputBitmap_t lastState)
{
    inputBitmap_t state = 0;
    BaseType_t now = xTaskGetTickCount();
    if (debounce > 0)
    {
        if ((now - debounce) >= DEBOUNCE_TICKS)
            debounce = 0;
        return lastState & mask;
    }

    for (uint8_t switchIndex = 0; switchIndex < switchCount; switchIndex++)
    {
        // Choose selector pins
        for (uint8_t selPinIndex = 0; selPinIndex < selectorPinCount; selPinIndex++)
            gpio_set_level(selectorPins[selPinIndex], switchIndex & (1 << selPinIndex));

        // Wait for the signal to propagate
        vTaskDelay(SIGNAL_CHANGE_DELAY_TICKS);

        uint8_t inputPinIndex = switchIndex >> selectorPinCount;
        int level = gpio_get_level(inputPins[inputPinIndex]);
        if (level ^ negativeLogic)
            state = state | BITMAP(buttonNumbersArray[switchIndex]);
    }

    return state;
}
