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
// Constructors
// ----------------------------------------------------------------------------

ButtonMatrixInput::ButtonMatrixInput(
    const gpio_num_t selectorPins[],
    const uint8_t selectorPinCount,
    const gpio_num_t inputPins[],
    const uint8_t inputPinCount,
    inputNumber_t *buttonNumbersArray,
    inputNumber_t alternateFirstInputNumber,
    DigitalPolledInput *nextInChain) : DigitalPolledInput(nextInChain)
{
    if ((selectorPinCount > MAX_MATRIX_SELECTOR_COUNT) ||
        (inputPinCount > MAX_MATRIX_INPUT_COUNT) ||
        (selectorPinCount == 0) ||
        (inputPinCount == 0))
    {
        log_e("Too few/many input or selector pins at ButtonMatrixInput::ButtonMatrixInput()");
        abort();
    }
    else
    {
        // Initialize debouncing state
        for (int r = 0; r < selectorPinCount; r++)
            for (int c = 0; c < inputPinCount; c++)
                debounce[r][c] = 0;

        // Check and initialize pins
        for (int i = 0; i < selectorPinCount; i++)
            checkAndInitializeSelectorPin(selectorPins[i]);
        for (int i = 0; i < inputPinCount; i++)
            checkAndInitializeInputPin(inputPins[i],true,false);
    }

    this->selectorPinCount = selectorPinCount;
    this->inputPinCount = inputPinCount;
    this->buttonNumbersArray = buttonNumbersArray;
    this->alternateFirstInputNumber = alternateFirstInputNumber;
    this->selectorPins = selectorPins;
    this->inputPins = inputPins;

    if (buttonNumbersArray != nullptr)
        updateMask(buttonNumbersArray, selectorPinCount * inputPinCount);
    else if (alternateFirstInputNumber <= MAX_INPUT_NUMBER)
        updateMask(selectorPinCount * inputPinCount, alternateFirstInputNumber);
    else
    {
        log_e("Unknown input numbers in call to ButtonMatrixInput::ButtonMatrixInput()");
        abort();
    }
}

// ----------------------------------------------------------------------------
// Polling
// ----------------------------------------------------------------------------

inputBitmap_t ButtonMatrixInput::read(inputBitmap_t lastState)
{
    inputBitmap_t state = 0;
    for (int selectorIndex = 0; selectorIndex < selectorPinCount; selectorIndex++)
    {
        gpio_set_level(selectorPins[selectorIndex], 1);
        // Wait for the signal to change from LOW to HIGH due to parasite capacitances.
        vTaskDelay(SIGNAL_CHANGE_DELAY_TICKS);
        for (int inputIndex = 0; inputIndex < inputPinCount; inputIndex++)
        {
            // Also valid: inputNumber_t n = (selectorIndex * inputPinCount) + inputIndex ;
            inputNumber_t n = (inputIndex * selectorPinCount) + selectorIndex;
            if (buttonNumbersArray == nullptr)
                n = n + alternateFirstInputNumber;
            else
                n = buttonNumbersArray[n];

            if (debounce[selectorIndex][inputIndex] > 0)
            {
                BaseType_t now = xTaskGetTickCount();
                if ((now - debounce[selectorIndex][inputIndex]) >= DEBOUNCE_TICKS)
                    debounce[selectorIndex][inputIndex] = 0;
                state = state | (lastState & BITMAP(n));
            }
            else
            {
                int level = gpio_get_level(inputPins[inputIndex]);
                if (level)
                {
                    state = state | BITMAP(n);
                }
            }
        }
        gpio_set_level(selectorPins[selectorIndex], 0);
        // Wait for the signal to change from HIGH to LOW.
        // Otherwise, there will be a false reading at the next iteration.
        vTaskDelay(SIGNAL_CHANGE_DELAY_TICKS);
    }
    return state;
}
