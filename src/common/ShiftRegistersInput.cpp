/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-02-19
 * @brief Use of PISO shift registers as inputs
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "ShiftRegistersInput.h"

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

#define SIGNAL_CHANGE_DELAY_TICKS 1

// ----------------------------------------------------------------------------
// Constructors
// ----------------------------------------------------------------------------

ShiftRegistersInput::ShiftRegistersInput(
    const gpio_num_t serialPin,
    const gpio_num_t loadPin,
    const gpio_num_t nextPin,
    const inputNumber_t *buttonNumbersArray,
    const uint8_t switchCount,
    const bool negativeLogic,
    const bool loadHighOrLow,
    const bool nextHighToLowOrLowToHigh,
    DigitalPolledInput *nextInChain) : DigitalPolledInput(nextInChain)
{
    this->serialPin = serialPin;
    this->loadPin = loadPin;
    this->nextPin = nextPin;
    this->buttonNumbersArray = buttonNumbersArray;
    this->switchCount = switchCount;
    this->negativeLogic = negativeLogic;
    this->loadHighOrLow = loadHighOrLow;
    this->nextHighToLowOrLowToHigh = nextHighToLowOrLowToHigh;

    if ((switchCount > 64) || (switchCount == 0))
    {
        log_e("Too few/many switches at ShiftRegistersInput::ShiftRegistersInput()");
        abort();
    }

    if (buttonNumbersArray != nullptr)
        updateMask(buttonNumbersArray, switchCount);
    else
    {
        log_e("Unknown input numbers in call to ShiftRegistersInput::ShiftRegistersInput()");
        abort();
    }

    // Initialize debouncing state
    debounce = (BaseType_t *)malloc(sizeof(BaseType_t) * switchCount);
    for (int i = 0; i < switchCount; i++)
        debounce[i] = 0;

    // Check and initialize pins
    checkAndInitializeInputPin(serialPin, false, false);
    checkAndInitializeSelectorPin(loadPin);
    checkAndInitializeSelectorPin(nextPin);
    gpio_set_level(nextPin, nextHighToLowOrLowToHigh);
}

ShiftRegistersInput::~ShiftRegistersInput()
{
    if (debounce)
        free(debounce);
}

// ----------------------------------------------------------------------------
// Polling
// ----------------------------------------------------------------------------

inputBitmap_t ShiftRegistersInput::read(inputBitmap_t lastState)
{
    inputBitmap_t state = 0;

    // Parallel load
    gpio_set_level(loadPin, loadHighOrLow);
    vTaskDelay(SIGNAL_CHANGE_DELAY_TICKS);
    gpio_set_level(loadPin, !loadHighOrLow);

    // Serial output
    for (uint8_t switchIndex = 0; switchIndex < switchCount; switchIndex++)
    {
        if (debounce[switchIndex])
        {
            BaseType_t now = xTaskGetTickCount();
            if ((now - debounce[switchIndex]) >= DEBOUNCE_TICKS)
                debounce[switchIndex] = 0;
            state = state | (lastState & BITMAP(switchIndex));
        }
        else
        {
            int level = gpio_get_level(serialPin);
            if (level ^ negativeLogic)
                state = state | BITMAP(buttonNumbersArray[switchIndex]);
        };

        // next
        gpio_set_level(nextPin, !nextHighToLowOrLowToHigh);
        vTaskDelay(SIGNAL_CHANGE_DELAY_TICKS);
        gpio_set_level(nextPin, nextHighToLowOrLowToHigh);
    }

    return state;
}
