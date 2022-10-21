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
// Constructor
// ----------------------------------------------------------------------------

ButtonMatrixInput::ButtonMatrixInput(
    int firstButtonNumber, 
    PolledInput *nextInChain): 
        PolledInput(firstButtonNumber,nextInChain)
{
    if (firstButtonNumber>63) {
        log_e("First button number is too high at ButtonMatrixInput's constructor");
        abort();
    }
    selectorPinCount = 0;
    inputPinCount = 0;
    for (int r = 0; r < MAX_MATRIX_SELECTOR_COUNT; r++)
        for (int c = 0; c < MAX_MATRIX_INPUT_COUNT; c++)
            debounce[r][c] = 0;
}

// ----------------------------------------------------------------------------
// Pin setup
// ----------------------------------------------------------------------------

void ButtonMatrixInput::addSelectorPin(gpio_num_t aPin)
{
    if (!GPIO_IS_VALID_OUTPUT_GPIO(aPin))
    {
        log_e("Requested GPIO %d at ButtonMatrixInput::addSelectorPin() can't be used as output",aPin);
        abort();
    }
    else if (selectorPinCount < MAX_MATRIX_SELECTOR_COUNT)
    {
        //ESP_ERROR_CHECK(gpio_set_direction(aPin, GPIO_MODE_OUTPUT));
        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = (1ULL<<aPin);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        ESP_ERROR_CHECK(gpio_config(&io_conf));

        gpio_set_level(aPin, 0);
        selectorPin[selectorPinCount++] = aPin;
        updateMask(inputPinCount * selectorPinCount);
    }
    else
    {
        log_e("Too many selector pins at ButtonMatrixInput::addSelectorPin()");
        abort();
    }
}

void ButtonMatrixInput::addInputPin(gpio_num_t aPin)
{
    if (!GPIO_IS_VALID_GPIO(aPin))
    {
        log_e("Requested GPIO %d at ButtonMatrixInput::addInputPin() can't be used as input",aPin);
        abort();
    }
    else if (inputPinCount < MAX_MATRIX_INPUT_COUNT)
    {
        //ESP_ERROR_CHECK(gpio_set_direction(aPin, GPIO_MODE_INPUT));
        //ESP_ERROR_CHECK(gpio_set_pull_mode(aPin, GPIO_PULLDOWN_ONLY));
        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pin_bit_mask = (1ULL<<aPin);
        io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        ESP_ERROR_CHECK(gpio_config(&io_conf));

        inputPin[inputPinCount++] = aPin;
        updateMask(inputPinCount * selectorPinCount);
    }
    else
    {
        log_e("Too many input pins at ButtonMatrixInput::addInputPin()");
        abort();
    }
}

// ----------------------------------------------------------------------------
// Polling
// ----------------------------------------------------------------------------

inputBitmap_t ButtonMatrixInput::read(inputBitmap_t lastState)
{
    inputBitmap_t state = 0;
    for (int selectorIndex = 0; selectorIndex < selectorPinCount ; selectorIndex++)
    {
        gpio_set_level(selectorPin[selectorIndex], 1);
        // Wait for the signal to change from LOW to HIGH due to parasite capacitances. 
        vTaskDelay(SIGNAL_CHANGE_DELAY_TICKS);
        for (int inputIndex = 0; inputIndex < inputPinCount; inputIndex++)
        {
            //inputNumber_t n = (selectorIndex * inputPinCount) + inputIndex + firstInputNumber;
            inputNumber_t n = (inputIndex * selectorPinCount) + selectorIndex + firstInputNumber;
            if (debounce[selectorIndex][inputIndex] > 0)
            {
                BaseType_t now = xTaskGetTickCount();
                if ((now - debounce[selectorIndex][inputIndex]) >= DEBOUNCE_TICKS)
                    debounce[selectorIndex][inputIndex] = 0;
                state = state | (lastState & BITMAP(n));
            }
            else
            {
                int level = gpio_get_level(inputPin[inputIndex]);
                if (level)
                {
                    state = state | BITMAP(n);
                }
            }
        }
        gpio_set_level(selectorPin[selectorIndex], 0);
        // Wait for the signal to change from HIGH to LOW. 
        // Otherwise, there will be a false reading at the next iteration.
        vTaskDelay(SIGNAL_CHANGE_DELAY_TICKS);
    }
    return state;
}
