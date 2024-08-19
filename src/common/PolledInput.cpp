/**
 * @file PolledInput.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Detect input events in a polling (or sampling) loop.
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "PolledInput.h"
#include "adcTools.h"
#include <climits>
#include <Arduino.h> // function map()
// #include <limits.h>
#include "pins_arduino.h"
#include "i2cTools.h"

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

void DigitalPolledInput::abortOnInvalidInputNumber(inputNumber_t number)
{
    if (number > MAX_INPUT_NUMBER)
    {
        log_e("Invalid input number %d (not in range 0-63)", number);
        abort();
    }
}

inputNumber_t DigitalPolledInput::computeMask(
    const inputNumber_t inputNumbersArray[],
    uint8_t inputsCount)
{
    inputNumber_t mask;
    mask = 0ULL;
    for (uint8_t i = 0; i < inputsCount; i++)
    {
        abortOnInvalidInputNumber(inputNumbersArray[i]);
        inputBitmap_t currentBitmap = BITMAP(inputNumbersArray[i]);
        mask = mask | currentBitmap;
    }
    mask = ~mask;
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
    // initialize
    this->pinNumber = pinNumber;
    this->pullupOrPulldown = pullupOrPulldown;
    abortOnInvalidInputNumber(buttonNumber);
    this->bitmap = BITMAP(buttonNumber);
    this->mask = ~(this->bitmap);

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
}

// ----------------------------------------------------------------------------
// Virtual methods
// ----------------------------------------------------------------------------

inputBitmap_t DigitalButton::read(inputBitmap_t lastState)
{
    int reading = gpio_get_level(pinNumber);
    if (reading ^ pullupOrPulldown)
        return bitmap;
    else
        return 0ULL;
}

// ============================================================================
// Implementation of class: AnalogAxisInput
// ============================================================================

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

AnalogAxisInput::AnalogAxisInput(
    gpio_num_t pinNumber,
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
    this->reversed = reversed;
    lastADCReading = 0;

    // Note: we assume the potentiometer works on the full range of voltage.
    // If that is not the case, the user should ask for recalibration.
    // Storage of calibration data is handled at `Inputs.cpp`
    minADCReading = 0;
    maxADCReading = 254;
}

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

void AnalogAxisInput::read(clutchValue_t &value, bool &autocalibrated)
{
    // read ADC and remove 4 bits of noise
    int currentReading = getADCreading(pinNumber, ADC_ATTEN_DB_11) >> 4;
    // filter
    currentReading = (currentReading + lastADCReading) >> 1; // average

    // Autocalibrate
    autocalibrated = false;
    if (currentReading < minADCReading)
    {
        minADCReading = currentReading;
        autocalibrated = true;
    }
    if (currentReading > maxADCReading)
    {
        maxADCReading = currentReading;
        autocalibrated = true;
    }

    // map ADC reading to axis value
    if (minADCReading == maxADCReading)
        value = CLUTCH_NONE_VALUE;
    else if (reversed)
        value = map(currentReading, minADCReading, maxADCReading, CLUTCH_FULL_VALUE, CLUTCH_NONE_VALUE);
    else
        value = map(currentReading, minADCReading, maxADCReading, CLUTCH_NONE_VALUE, CLUTCH_FULL_VALUE);
    lastADCReading = currentReading;
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

// ============================================================================
// Implementation of class: I2CInput
// ============================================================================

// ----------------------------------------------------------------------------
// Addresses
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

I2CInput::I2CInput(
    uint8_t address7bits,
    bool useSecondaryBus,
    uint8_t max_speed_mult,
    DigitalPolledInput *nextInChain) : DigitalPolledInput(nextInChain)
{
    i2c::abortOnInvalidAddress(address7bits);
    deviceAddress = (address7bits << 1);
    busDriver = useSecondaryBus ? I2C_NUM_1 : I2C_NUM_0;
    i2c::require(max_speed_mult, useSecondaryBus);
    if (!i2c::probe(address7bits, useSecondaryBus))
    {
        log_e("I2C device not found at address %x, bus=%d", address7bits, busDriver);
        abort();
    }
}

// ============================================================================
// Implementation of class: I2CButtonsInput
// ============================================================================

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

I2CButtonsInput::I2CButtonsInput(
    uint8_t buttonsCount,
    uint8_t address7Bits,
    bool useSecondaryBus,
    uint8_t max_speed_mult,
    DigitalPolledInput *nextInChain) : I2CInput(address7Bits, useSecondaryBus, max_speed_mult, nextInChain)
{
    if (buttonsCount > MAX_EXPANDED_GPIO_COUNT)
    {
        log_e("Too many buttons at GPIO expander. Address=%x, bus=%d", address7Bits, busDriver);
        abort();
    }
    gpioCount = buttonsCount;
    for (uint8_t i = 0; i < MAX_EXPANDED_GPIO_COUNT; i++)
        gpioBitmap[i] = 0ULL;
}

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

inputBitmap_t I2CButtonsInput::read(inputBitmap_t lastState)
{
    inputBitmap_t GPIOstate;
    if (getGPIOstate(GPIOstate))
    {
        // GPIOstate = !GPIOstate; // Note: all buttons work in negative logic
        inputBitmap_t result = 0ULL;
        for (inputBitmap_t i = 0; i < gpioCount; i++)
        {
            if (GPIOstate & BITMAP(i))
                result |= gpioBitmap[i];
        }
        return result;
    }
    return lastState;
}