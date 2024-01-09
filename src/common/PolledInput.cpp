/**
 * @file PolledInput.cpp
 *
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
#include <Arduino.h> // function map()
// #include <limits.h>

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
    DigitalPolledInput *nextInChain) : PolledInput(nextInChain)
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
        log_e("Last button number is too high at DigitalPolledInput instance");
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
            log_e("Invalid input number at DigitalPolledInput::updateMask()");
            abort();
        }
        inputBitmap_t currentBitmap = BITMAP(inputNumbersArray[i]);
        mask = mask | currentBitmap;
    }
    mask = ~mask;
}

// ----------------------------------------------------------------------------
// Pin setup
// ----------------------------------------------------------------------------

void DigitalPolledInput::checkAndInitializeSelectorPin(gpio_num_t aPin)
{
    if (!GPIO_IS_VALID_OUTPUT_GPIO(aPin))
    {
        log_e("Requested GPIO %d can't be used as output", aPin);
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

void DigitalPolledInput::checkAndInitializeInputPin(gpio_num_t aPin, bool enablePullDown, bool enablePullUp)
{
    if (!GPIO_IS_VALID_GPIO(aPin))
    {
        log_e("Requested GPIO %d can't be used as input", aPin);
        abort();
    }
    else
    {
        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pin_bit_mask = (1ULL << aPin);
        if (enablePullDown)
            io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
        else
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        if (enablePullUp)
            io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        else
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        ESP_ERROR_CHECK(gpio_config(&io_conf));
    }
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
        mask &= (firstInChain->mask);
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
    this->bitmap = BITMAP(buttonNumber);
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
    inputNumber_t inputNumber,
    bool reversed)
{
    // Check parameters
    if (!GPIO_IS_VALID_GPIO(pinNumber) ||
        (digitalPinToAnalogChannel(pinNumber) < 0))
    {
        log_e("AnalogAxisInput::AnalogAxisInput: given pins are not usable");
        abort();
    }

    // Initialize
    this->pinNumber = pinNumber;
    this->bitmap = BITMAP(inputNumber);
    this->mask = BITMASK(1, inputNumber);
    this->reversed = reversed;
    lastADCReading = 0;
    lastValue = -128;

    // Note: we assume the potentiometer works on the full range of voltage.
    // If that is not the case, the user should ask for recalibration.
    // Storage of calibration data is handled at `Inputs.cpp`
    minADCReading = 0;
    maxADCReading = 254;
}

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

void AnalogAxisInput::read(clutchValue_t *value, bool *changed, bool *autocalibrated)
{
    // read ADC and remove 4 bits of noise
    int currentReading = getADCreading(pinNumber, ADC_ATTEN_DB_11) >> 4;
    // filter
    currentReading = (currentReading + lastADCReading) >> 1; // average

    // Autocalibrate
    *autocalibrated = false;
    if (currentReading < minADCReading)
    {
        minADCReading = currentReading;
        *autocalibrated = true;
    }
    if (currentReading > maxADCReading)
    {
        maxADCReading = currentReading;
        *autocalibrated = true;
    }

    // map ADC reading to axis value
    if (reversed)
        *value = map(currentReading, minADCReading, maxADCReading, CLUTCH_FULL_VALUE, CLUTCH_NONE_VALUE);
    else
        *value = map(currentReading, minADCReading, maxADCReading, CLUTCH_NONE_VALUE, CLUTCH_FULL_VALUE);
    *changed = ((*value) != lastValue);
    lastADCReading = currentReading;
    lastValue = *value;
}

void AnalogAxisInput::getCalibrationData(int *minReading, int *maxReading)
{
    *minReading = this->minADCReading;
    *maxReading = this->maxADCReading;
}

void AnalogAxisInput::resetCalibrationData()
{
    minADCReading = INT_MAX;
    maxADCReading = INT_MIN;
}

void AnalogAxisInput::setCalibrationData(int minReading, int maxReading)
{
    minADCReading = minReading;
    maxADCReading = maxReading;
}
