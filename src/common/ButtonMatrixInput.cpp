/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-03
 * @brief Input from a switch/button matrix
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "ButtonMatrixInput.h"
#include <Arduino.h>

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

#define SIGNAL_CHANGE_DELAY_TICKS 5

// ----------------------------------------------------------------------------
// Constructor / destructor
// ----------------------------------------------------------------------------

ButtonMatrixInput::ButtonMatrixInput(
    const gpio_num_array_t selectorPins,
    const gpio_num_array_t inputPins,
    DigitalPolledInput *nextInChain) : DigitalPolledInput(nextInChain)
{
    selectorPinCount = selectorPins.size();
    inputPinCount = inputPins.size();
    if ((selectorPinCount > MAX_MATRIX_SELECTOR_COUNT) ||
        (inputPinCount > MAX_MATRIX_INPUT_COUNT) ||
        (selectorPinCount == 0) ||
        (inputPinCount == 0))
    {
        log_e("Too few/many input or selector pins at ButtonMatrixInput::ButtonMatrixInput()");
        abort();
    }

    // Check and initialize pins
    for (int i = 0; i < selectorPinCount; i++)
    {
        this->selectorPins[i] = selectorPins[i];
        checkAndInitializeSelectorPin(selectorPins[i]);
    }
    for (int i = 0; i < inputPinCount; i++)
    {
        this->inputPins[i] = inputPins[i];
        checkAndInitializeInputPin(inputPins[i], true, false);
    }

    // Other initialization
    uint8_t switchCount = selectorPinCount * inputPinCount;
    bitmap = (inputBitmap_t *)malloc(sizeof(inputBitmap_t) * switchCount);
    if (!bitmap)
    {
        log_e("Not enough memory at ButtonMatrixInput::ButtonMatrixInput()");
        abort();
    }
    for (uint8_t i = 0; i < switchCount; i++)
        bitmap[i] = 0ULL;
}

ButtonMatrixInput::~ButtonMatrixInput()
{
    free(bitmap);
}

// ----------------------------------------------------------------------------
// Polling
// ----------------------------------------------------------------------------

inputBitmap_t ButtonMatrixInput::read(inputBitmap_t lastState)
{
    inputBitmap_t state = 0ULL;
    for (int selectorIndex = 0; selectorIndex < selectorPinCount; selectorIndex++)
    {
        gpio_set_level(selectorPins[selectorIndex], 1);
        // Wait for the signal to change from LOW to HIGH due to parasite capacitances.
        vTaskDelay(SIGNAL_CHANGE_DELAY_TICKS);
        for (int inputIndex = 0; inputIndex < inputPinCount; inputIndex++)
        {
            inputNumber_t n = (inputIndex * selectorPinCount) + selectorIndex;
            int level = gpio_get_level(inputPins[inputIndex]);
            if (level)
                state = state | bitmap[n];
        }
        gpio_set_level(selectorPins[selectorIndex], 0);
        // Wait for the signal to change from HIGH to LOW.
        // Otherwise, there will be a false reading at the next iteration.
        vTaskDelay(SIGNAL_CHANGE_DELAY_TICKS);
    }
    return state;
}

// ----------------------------------------------------------------------------
// Input numbers
// ----------------------------------------------------------------------------

ButtonMatrixInputSpec &ButtonMatrixInput::inputNumber(
    gpio_num_t selectorPin,
    gpio_num_t inputPin,
    inputNumber_t number)
{
    // locate index of selector pin
    uint8_t selectorPinIndex = 0;
    while ((selectorPinIndex < selectorPinCount) && (selectorPins[selectorPinIndex] != selectorPin))
        selectorPinIndex++;
    if (selectorPinIndex >= selectorPinCount)
    {
        // Not found
        log_e("Invalid selector pin at ButtonMatrixInput::inputNumber(%d,%d,%d)", selectorPin, inputPin, number);
        abort();
    }

    // locate index of input pin
    uint8_t inputPinIndex = 0;
    while ((inputPinIndex < inputPinCount) && (inputPins[inputPinIndex] != inputPin))
        inputPinIndex++;
    if (inputPinIndex >= inputPinCount)
    {
        // Not found
        log_e("Invalid input pin at ButtonMatrixInput::inputNumber(%d,%d,%d)", selectorPin, inputPin, number);
        abort();
    }

    // Set bitmap
    uint8_t index = (inputPinIndex * selectorPinCount) + selectorPinIndex;
    mask |= bitmap[index];
    bitmap[index] = BITMAP(number);
    mask &= ~bitmap[index];
    return *this;
}
