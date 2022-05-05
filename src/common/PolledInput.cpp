/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Detect input events in a polling (or sampling) loop.
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "PolledInput.h"
//#include <FreeRTOS.h>

// ============================================================================
// Implementation of class: PolledInput
// ============================================================================

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

PolledInput::PolledInput(
    inputNumber_t firstInputNumber,
    PolledInput *nextInChain)
{
    // Param check
    if (firstInputNumber > 63)
    {
        log_e("ERROR at PolledInput: button number out of range = %d",firstInputNumber);
        abort();
    }
    
    // Initialization
    this->firstInputNumber = firstInputNumber;
    this->nextInChain = nextInChain;
    this->mask = ~(0ULL);
}

// ----------------------------------------------------------------------------
// Getters
// ----------------------------------------------------------------------------

PolledInput *PolledInput::getNextInChain()
{
    return nextInChain;
}

// ----------------------------------------------------------------------------
// Setters
// ----------------------------------------------------------------------------

void PolledInput::updateMask(uint8_t inputsCount)
{
    if (inputsCount > (64 - firstInputNumber))
    {
        log_e("Last button number is too high at PolledInput instance");
        abort();
    }
    mask = BITMASK(inputsCount, firstInputNumber);
}

// ----------------------------------------------------------------------------
// Class methods
// ----------------------------------------------------------------------------

bool PolledInput::contains(
    PolledInput *instance,
    PolledInput *firstInChain)
{
    bool found = false;
    while (!found && (firstInChain != nullptr))
    {
        found = (firstInChain == instance);
        firstInChain = firstInChain->nextInChain;
    }
    return found;
}

inputBitmap_t PolledInput::readInChain(
    inputBitmap_t lastState,
    PolledInput *firstInChain)
{
    inputBitmap_t newState = 0ULL;
    while (firstInChain != nullptr)
    {
        inputBitmap_t itemState = firstInChain->read(lastState);
        newState = (newState & firstInChain->mask) | itemState;
        firstInChain = firstInChain->nextInChain;
    }
    return newState;
}

inputBitmap_t PolledInput::getChainMask(PolledInput *firstInChain)
{
    inputBitmap_t mask = ~(0ULL);
    while (firstInChain != nullptr)
    {
        mask &= firstInChain->mask;
        firstInChain = firstInChain->nextInChain;
    }
    return mask;
}

// ============================================================================
// Implementation of class: DigitalButton
// ============================================================================

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

DigitalButton::DigitalButton(
    gpio_num_t pinNumber,
    inputNumber_t buttonNumber,
    bool pullupOrPulldown,
    bool enableInternalPull,
    PolledInput *nextInChain) : PolledInput(buttonNumber, nextInChain)
{
    // initalize
    updateMask(1);
    this->pinNumber = pinNumber;
    this->pullupOrPulldown = pullupOrPulldown;
    bitmap = BITMAP(firstInputNumber);
    debouncing = false;

    // Pin setup
    ESP_ERROR_CHECK(gpio_set_direction(pinNumber, GPIO_MODE_INPUT));
    if (enableInternalPull)
    {
        if (pullupOrPulldown)
        {
            ESP_ERROR_CHECK(gpio_set_pull_mode(pinNumber, GPIO_PULLUP_ONLY));
        }
        else
        {
            ESP_ERROR_CHECK(gpio_set_pull_mode(pinNumber, GPIO_PULLDOWN_ONLY));
        }
    }
    else
        ESP_ERROR_CHECK(gpio_set_pull_mode(pinNumber, GPIO_FLOATING));
};

// ----------------------------------------------------------------------------
// Virtual methods
// ----------------------------------------------------------------------------

inputBitmap_t DigitalButton::read(inputBitmap_t lastState)
{
    inputBitmap_t state;
    if (debouncing)
    {
        debouncing = false;
        state = lastState & bitmap;
    }
    else
    {
        debouncing = true;
        int reading = gpio_get_level(pinNumber);
        if ((reading && !pullupOrPulldown) || (!reading && pullupOrPulldown))
            state = bitmap;
        else
            state = 0;
    }
    return state;
}

// ============================================================================
// Implementation of class: AnalogInput
// ============================================================================

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

AnalogInput::AnalogInput(
    gpio_num_t pinNumber,
    inputNumber_t firstInputNumber,
    analogReading_t *minReading,
    analogReading_t *maxReading,
    uint8_t arrayLength,
    PolledInput *nextInChain) : PolledInput(firstInputNumber, nextInChain)
{
    // Pin setup
    ESP_ERROR_CHECK(gpio_set_direction(pinNumber, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_pull_mode(pinNumber, GPIO_FLOATING));

    // Initialization
    this->pinNumber = pinNumber;
    this->arrayLength = arrayLength;
    this->minReading = minReading;
    this->maxReading = maxReading;
}

// ----------------------------------------------------------------------------
// Read
// ----------------------------------------------------------------------------

int AnalogInput::filteredAnalogRead()
{
    analogRead(pinNumber); // discard first reading
    int filterOutput = 0;
    for (int i = 0; i < 4; i++)
    {
        int reading = analogRead(pinNumber);
        // filterOutput = ((reading - filterOutput) / 4) + filterOutput;
        filterOutput = ((reading - filterOutput) >> 2) + filterOutput;
    }
    return filterOutput;
}

int AnalogInput::getReadingIndex()
{
    analogReading_t reading = filteredAnalogRead();

    for (inputNumber_t i = 0; i < arrayLength; i++)
    {
        if ((minReading[i] <= reading) && (reading <= maxReading[i]))
            return i;
    }

    // unknown
    return -1;
}