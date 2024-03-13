/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-01-19
 * @brief Use of analog multiplexors as inputs
 *
 * @copyright Licensed under the EUPL
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
    const gpio_num_array_t &selectorPins,
    const gpio_num_array_t &inputPins,
    const bool negativeLogic,
    DigitalPolledInput *nextInChain) : DigitalPolledInput(nextInChain)
{
    // Check and initialize pins
    selectorPinCount = selectorPins.size();
    inputPinCount = inputPins.size();
    if ((selectorPinCount == 0) || (inputPinCount == 0))
    {
        log_e("No input/selector pins at AnalogMultiplexerInput::AnalogMultiplexerInput()");
        abort();
    }
    switchCount = (1 << selectorPinCount) * inputPinCount;
    this->selectorPins = (gpio_num_t *)malloc(sizeof(gpio_num_t) * selectorPinCount);
    this->inputPins = (gpio_num_t *)malloc(sizeof(gpio_num_t) * inputPinCount);
    bitmap = (inputBitmap_t *)malloc(sizeof(inputBitmap_t) * switchCount);
    if (!(this->selectorPins) || !(this->inputPins) || !bitmap)
    {
        log_e("Not enough memory at AnalogMultiplexerInput::AnalogMultiplexerInput()");
        abort();
    }
    for (int i = 0; i < selectorPinCount; i++)
    {
        this->selectorPins[i] = selectorPins[i];
        checkAndInitializeSelectorPin(this->selectorPins[i]);
    }
    for (int i = 0; i < inputPinCount; i++)
    {
        this->inputPins[i] = inputPins[i];
        checkAndInitializeInputPin(inputPins[i], !negativeLogic, negativeLogic);
    }

    // Other initialization
    this->negativeLogic = negativeLogic;
    for (int i = 0; i < switchCount; i++)
        bitmap[i] = 0ULL;
}

AnalogMultiplexerInput::~AnalogMultiplexerInput()
{
    free(selectorPins);
    free(inputPins);
    free(bitmap);
}

// ----------------------------------------------------------------------------
// Polling
// ----------------------------------------------------------------------------

inputBitmap_t AnalogMultiplexerInput::read(inputBitmap_t lastState)
{
    inputBitmap_t state = 0ULL;

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
            state = state | bitmap[switchIndex];
    };

    return state;
}

// ----------------------------------------------------------------------------
// Input numbers
// ----------------------------------------------------------------------------

Multiplexers8InputSpec &AnalogMultiplexerInput::inputNumber(
    gpio_num_t inputPin,
    mux8_pin_t pin,
    inputNumber_t number)
{
    uint8_t muxPinIndex = static_cast<uint8_t>(pin);

    // locate index of input pin
    uint8_t inputPinIndex = 0;
    while ((inputPinIndex < inputPinCount) && (inputPins[inputPinIndex] != inputPin))
        inputPinIndex++;
    if (inputPinIndex >= inputPinCount)
    {
        // Not found
        log_e("Invalid input pin at AnalogMultiplexerInput::inputNumber(%d,%d,%d)", inputPin, muxPinIndex, number);
        abort();
    }

    // Set bitmap
    uint8_t index = ((1 << selectorPinCount) * inputPinIndex) + muxPinIndex;
    mask |= bitmap[index];
    bitmap[index] = BITMAP(number);
    mask &= ~bitmap[index];
    return *this;
}