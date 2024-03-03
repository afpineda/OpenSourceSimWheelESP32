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

void DigitalPolledInput::updateMask(const inputNumber_t *inputNumbersArray, uint8_t inputsCount)
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
    // initialize
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
    if (reversed)
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

static bool isPrimaryBusInitialized = false;

// ----------------------------------------------------------------------------
// Driver initialization
// ----------------------------------------------------------------------------

void I2CInput::initializePrimaryBus(bool useFastClock)
{
    i2c_config_t conf;
    memset(&conf, 0, sizeof(i2c_config_t));
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA;
    conf.scl_io_num = SCL;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000;
    if (useFastClock)
        conf.master.clk_speed *= 4;
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
    isPrimaryBusInitialized = true;
}

void I2CInput::initializeSecondaryBus(gpio_num_t sdaPin, gpio_num_t sclPin, bool useFastClock)
{
    i2c_config_t conf;
    memset(&conf, 0, sizeof(i2c_config_t));
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sdaPin;
    conf.scl_io_num = sclPin;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000;
    if (useFastClock)
        conf.master.clk_speed *= 4;
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_1, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_1, I2C_MODE_MASTER, 0, 0, 0));
}

// ----------------------------------------------------------------------------
// I2C probe
// ----------------------------------------------------------------------------

bool I2CInput::probe(uint8_t address7bits, i2c_port_t bus)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address7bits << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    bool result = (i2c_master_cmd_begin(bus, cmd, 500 / portTICK_RATE_MS) == ESP_OK);
    i2c_cmd_link_delete(cmd);
    return result;
}

bool I2CInput::hardwareAddr2FullAddress(
    uint8_t address3bits,
    i2c_port_t bus,
    uint8_t &address7bits)
{
    address3bits = address3bits & 0b00000111;
    uint8_t count = 0;
    for (uint8_t other4bits = 0; other4bits < 16; other4bits++)
    {
        uint8_t tryAddress = (other4bits << 3) | address3bits;
        if (probe(tryAddress, bus)) {
            address7bits = tryAddress;
            count++;
        }
    }
    return (count==1);
}

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

I2CInput::I2CInput(
    uint8_t address7bits,
    bool useSecondaryBus,
    DigitalPolledInput *nextInChain) : DigitalPolledInput(nextInChain)
{
    deviceAddress = (address7bits << 1);
    busDriver = I2CInput::getBusDriver(useSecondaryBus);
    if (!useSecondaryBus && !isPrimaryBusInitialized)
        initializePrimaryBus();

    if (!probe(address7bits, busDriver))
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
    const inputNumber_t *buttonNumbersArray,
    uint8_t address7Bits,
    bool useSecondaryBus,
    DigitalPolledInput *nextInChain) : I2CInput(address7Bits, useSecondaryBus, nextInChain)
{
    if (buttonsCount > MAX_EXPANDED_GPIO_COUNT)
    {
        log_e("Too many buttons at GPIO expander. Address=%x, bus=%d", address7Bits, busDriver);
        abort();
    }
    updateMask(buttonNumbersArray, buttonsCount);
    gpioCount = buttonsCount;
    for (uint8_t i = 0; i < buttonsCount; i++)
    {
        gpioBitmap[i] = BITMAP(buttonNumbersArray[i]);
    }
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