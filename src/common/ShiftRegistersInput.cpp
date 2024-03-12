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
    const uint8_t switchCount,
    const bool negativeLogic,
    const bool loadHighOrLow,
    const bool nextHighToLowOrLowToHigh,
    DigitalPolledInput *nextInChain) : DigitalPolledInput(nextInChain)
{
    this->serialPin = serialPin;
    this->loadPin = loadPin;
    this->nextPin = nextPin;
    this->switchCount = switchCount;
    this->negativeLogic = negativeLogic;
    this->loadHighOrLow = loadHighOrLow;
    this->nextHighToLowOrLowToHigh = nextHighToLowOrLowToHigh;
    if ((switchCount > 64) || (switchCount == 0))
    {
        log_e("Too few/many switches at ShiftRegistersInput::ShiftRegistersInput()");
        abort();
    }

    // Initialize bitmaps
    bitmap = (inputBitmap_t *)malloc(sizeof(inputBitmap_t) * switchCount);
    if (!bitmap)
    {
        log_e("Unable to allocate memory for input bitmaps at ShiftRegistersInput::ShiftRegistersInput()");
        abort();
    }
    for (int i = 0; i < switchCount; i++)
        bitmap[i] = 0ULL;

    // Check and initialize pins
    checkAndInitializeInputPin(serialPin, false, false);
    checkAndInitializeSelectorPin(loadPin);
    checkAndInitializeSelectorPin(nextPin);
    gpio_set_level(nextPin, nextHighToLowOrLowToHigh);
}

ShiftRegistersInput::~ShiftRegistersInput()
{
    free(bitmap);
}

// ----------------------------------------------------------------------------
// Polling
// ----------------------------------------------------------------------------

inputBitmap_t ShiftRegistersInput::read(inputBitmap_t lastState)
{
    inputBitmap_t state = 0ULL;

    // Parallel load
    gpio_set_level(loadPin, loadHighOrLow);
    vTaskDelay(SIGNAL_CHANGE_DELAY_TICKS);
    gpio_set_level(loadPin, !loadHighOrLow);

    // Serial output
    for (uint8_t switchIndex = 0; switchIndex < switchCount; switchIndex++)
    {
        int level = gpio_get_level(serialPin);
        if (level ^ negativeLogic)
            state = state | bitmap[switchIndex];

        // next
        gpio_set_level(nextPin, !nextHighToLowOrLowToHigh);
        vTaskDelay(SIGNAL_CHANGE_DELAY_TICKS);
        gpio_set_level(nextPin, nextHighToLowOrLowToHigh);
    }

    return state;
}

ShiftRegisters8InputSpec &ShiftRegistersInput::inputNumber(
    uint8_t indexInChain,
    sr8_pin_t pin,
    inputNumber_t number)
{
    abortOnInvalidInputNumber(number);
    uint16_t index = (indexInChain * 8) + static_cast<uint8_t>(pin);
    if ((pin==sr8_pin_t::SER) && (index!=switchCount-1))
        index = switchCount;
    if (index >= switchCount) {
        log_e("Invalid position of input number %d at shift register index %d and pin %d",
            number,
            indexInChain,
            static_cast<uint8_t>(pin));
        abort();
    }
    mask |= bitmap[index];
    bitmap[index] = BITMAP(number);
    mask &= ~bitmap[index];
    return *this;
}