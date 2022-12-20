/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Detect input events in a polling (or sampling) loop.
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include "PolledInput.h"
#include "adcTools.h"
#include <climits>
//#include <limits.h>


// ============================================================================
// Implementation of class: PolledInput
// ============================================================================

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

PolledInput::PolledInput(
    PolledInput *nextInChain)
{
    this->nextInChain = nextInChain;
}

// ----------------------------------------------------------------------------
// Getters
// ----------------------------------------------------------------------------

PolledInput *PolledInput::getNextInChain()
{
    return nextInChain;
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

// ============================================================================
// Implementation of class: DigitalPolledInput
// ============================================================================

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

DigitalPolledInput::DigitalPolledInput(
    DigitalPolledInput *nextInChain): PolledInput(nextInChain)
{
    this->mask = ~(0ULL);
}

// ----------------------------------------------------------------------------
// Setters
// ----------------------------------------------------------------------------

void DigitalPolledInput::updateMask(uint8_t inputsCount, inputNumber_t firstInputNumber)
{
    if (inputsCount > (64 - firstInputNumber))
    {
        log_e("Last button number is too high at PolledInput instance");
        abort();
    }
    mask = BITMASK(inputsCount, firstInputNumber);
}

void DigitalPolledInput::updateMask(inputNumber_t *inputNumbersArray, uint8_t inputsCount)
{
    if (inputNumbersArray == nullptr)
    {
        mask = ~0ULL;
        return;
    }

    mask = 0ULL;
    for (uint8_t i = 0; i < inputsCount; i++)
    {
        if (inputNumbersArray[i] > MAX_INPUT_NUMBER)
        {
            log_e("Invalid input number at PolledInput::updateMask()");
            abort();
        }
        inputBitmap_t currentBitmap = BITMAP(inputNumbersArray[i]);
        mask = mask | currentBitmap;
    }
    mask = ~mask;
}

// ----------------------------------------------------------------------------
// Class methods
// ----------------------------------------------------------------------------

inputBitmap_t DigitalPolledInput::readInChain(
    inputBitmap_t lastState,
    DigitalPolledInput *firstInChain)
{
    inputBitmap_t newState = 0ULL;
    while (firstInChain != nullptr)
    {
        inputBitmap_t itemState = firstInChain->read(lastState);
        newState = (newState & firstInChain->mask) | itemState;
        firstInChain = (DigitalPolledInput *)(firstInChain->nextInChain);
    }
    return newState;
}

inputBitmap_t DigitalPolledInput::getChainMask(DigitalPolledInput *firstInChain)
{
    inputBitmap_t mask = ~(0ULL);
    while (firstInChain != nullptr)
    {
        mask &= ((DigitalPolledInput *)firstInChain)->mask;
        firstInChain = (DigitalPolledInput *)(firstInChain->nextInChain);
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
    DigitalPolledInput *nextInChain) : DigitalPolledInput(nextInChain)
{
    // initalize
    updateMask(1, buttonNumber);
    this->pinNumber = pinNumber;
    this->pullupOrPulldown = pullupOrPulldown;
    bitmap = BITMAP(buttonNumber);
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
// Implementation of class: AnalogAxisInput
// ============================================================================

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

AnalogAxisInput::AnalogAxisInput(
        gpio_num_t pinNumber,
        uint8_t axisIndex,
        AnalogAxisInput *nextInChain): PolledInput(nextInChain)
{
    if (!GPIO_IS_VALID_GPIO(pinNumber) ||
        (digitalPinToAnalogChannel(pinNumber) < 0))
    {
        log_e("AnalogAxisInput::AnalogAxisInput: given pins are not usable");
        abort();
    }
    this->pinNumber = pinNumber;
    this->axisIndex = axisIndex;
    minADCReading = INT_MAX;
    maxADCReading = INT_MIN;
    lastADCReading = 0;
}

// ----------------------------------------------------------------------------
// virtual methods
// ----------------------------------------------------------------------------

void AnalogAxisInput::read(uint8_t *axisIndex, clutchValue_t *value)
{
    *axisIndex = this->axisIndex;
    // read ADC and remove 4 bits of noise
    int currentReading = getADCreading(pinNumber,ADC_ATTEN_DB_11) << 4;
    // filter
    currentReading = (currentReading + lastADCReading) << 2;

    // Autocalibrate
    if (currentReading < minADCReading)
    {
        minADCReading = currentReading;
    }
    if (currentReading > maxADCReading)
    {
        maxADCReading = currentReading;
    }

    // map ADC reading to axis value
    *value = map(currentReading, minADCReading, maxADCReading, CLUTCH_NONE_VALUE, CLUTCH_FULL_VALUE);
}
