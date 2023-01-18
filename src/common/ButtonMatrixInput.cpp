/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-03
 * @brief Input from a switch/button matrix
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
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
    if ((selectorPinCount > MAX_MATRIX_SELECTOR_COUNT) || (inputPinCount > MAX_MATRIX_INPUT_COUNT))
    {
        log_e("Too many input or selector pins at ButtonMatrixInput::ButtonMatrixInput()");
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
            checkAndInitializeInputPin(inputPins[i]);
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
// Pin setup
// ----------------------------------------------------------------------------

void ButtonMatrixInput::checkAndInitializeSelectorPin(gpio_num_t aPin)
{
    if (!GPIO_IS_VALID_OUTPUT_GPIO(aPin))
    {
        log_e("Requested GPIO %d at ButtonMatrixInput can't be used as output", aPin);
        abort();
    }
    else
    {
        // ESP_ERROR_CHECK(gpio_set_direction(aPin, GPIO_MODE_OUTPUT));
        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = (1ULL << aPin);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        ESP_ERROR_CHECK(gpio_config(&io_conf));
        gpio_set_level(aPin, 0);
    }
}

void ButtonMatrixInput::checkAndInitializeInputPin(gpio_num_t aPin)
{
    if (!GPIO_IS_VALID_GPIO(aPin))
    {
        log_e("Requested GPIO %d at ButtonMatrixInput can't be used as input", aPin);
        abort();
    }
    else
    {
        // ESP_ERROR_CHECK(gpio_set_direction(aPin, GPIO_MODE_INPUT));
        // ESP_ERROR_CHECK(gpio_set_pull_mode(aPin, GPIO_PULLDOWN_ONLY));
        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pin_bit_mask = (1ULL << aPin);
        io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        ESP_ERROR_CHECK(gpio_config(&io_conf));
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
