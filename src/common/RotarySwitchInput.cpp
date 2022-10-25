/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-03
 * @brief Input from an absolute rotary switch using a voltage ladder
 * @note **Warning**: This code has not been tested. It is not used since the underlying
 *       circuit is not suitable for this project.
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "RotarySwitchInput.h"

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

RotarySwitchInput::RotarySwitchInput(
    gpio_num_t pinNumber,
    inputNumber_t firstInputNumber,
    uint8_t inputCount,
    analogReading_t *minReading,
    analogReading_t *maxReading,
    PolledInput *nextInChain = nullptr):
        AnalogInput(pinNumber,firstInputNumber,minReading,maxReading,inputCount,nextInChain)
{
    debouncing = false;
    lastIndex = -1;
    updateMask(inputCount);
}

// ----------------------------------------------------------------------------
// Reading
// ----------------------------------------------------------------------------

inputBitmap_t RotarySwitchInput::read(inputBitmap_t lastState)
{
    inputBitmap_t state;
    if (debouncing)
    {
        debouncing = false;
        state = lastState & ~mask;
    }
    else
    {
        debouncing = true;
        int index = getReadingIndex();
        if (index >= 0) {
            if (index!=lastIndex) {
                lastIndex = index;
                state = BITMAP(index+firstInputNumber);
            } else
                state = 0;
        } else
            state = lastState & ~mask;
    }
    return state;
}