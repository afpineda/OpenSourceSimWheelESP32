/**
 * @file InputHardware.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-02
 * @brief Input hardware classes
 *
 * @copyright Licensed under the EUPL
 *
 */

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "InputHardware.hpp"
#include "HAL.hpp"
#include "esp32-hal.h"   // For portSET_INTERRUPT_MASK_FROM_ISR
#include "driver/gpio.h" // For gpio_set_level/gpio_get_level()

//-------------------------------------------------------------------
// Globals
//-------------------------------------------------------------------

// I2C
#define I2C_TIMEOUT_MS 30

// MCP23017 registers
#define MCP23017_IO_CONFIGURATION 0x0A
#define MCP23017_IO_DIRECTION 0x00
#define MCP23017_PULL_UP_RESISTORS 0x0C
#define MCP23017_GPIO 0x12
#define MCP23017_POLARITY 0x02
#define MCP23017_INTERRUPT_ON_CHANGE 0x04
#define MCP23017_INTERRUPT_CONTROL 0x08
#define MCP23017_INTERRUPT_DEFAULT_VALUE 0x06

// Active wait
#define signal_change_delay(n) active_wait_ns(n)

//-------------------------------------------------------------------
// Single button
//-------------------------------------------------------------------

DigitalButton::DigitalButton(
    InputGPIO pinNumber,
    InputNumber buttonNumber) : DigitalInput()
{
    internals::hal::gpio::forInput(pinNumber, false, true);
    this->pinNumber = pinNumber;
    this->inputNumber = buttonNumber;
}

//-------------------------------------------------------------------

void DigitalButton::read(uint128_t &state)
{
    int reading = GPIO_GET_LEVEL(pinNumber);
    // Note: using negative logic (pulled up)
    state.set_bit(inputNumber, reading ? false : true);
}

//-------------------------------------------------------------------
// Rotary encoder
//-------------------------------------------------------------------

// Note: IRAM_ATTR no longer needed

void RotaryEncoderInput::isrh(void *instance)
{
    static const uint8_t valid_code[] =
        {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

    // UBaseType_t lock = taskENTER_CRITICAL_FROM_ISR();
    UBaseType_t lock = portSET_INTERRUPT_MASK_FROM_ISR();
    RotaryEncoderInput *rotary = (RotaryEncoderInput *)instance;
    int clk = GPIO_GET_LEVEL(rotary->clkPin);
    int dt = GPIO_GET_LEVEL(rotary->dtPin);
    portCLEAR_INTERRUPT_MASK_FROM_ISR(lock);
    // taskEXIT_CRITICAL_FROM_ISR(lock);

    rotary->code <<= 2;
    rotary->code = rotary->code | (dt << 1) | clk;
    rotary->code &= 0x0f;

    if (valid_code[rotary->code])
    {
        rotary->sequence <<= 4;
        rotary->sequence |= rotary->code;
        uint16_t aux = rotary->sequence & 0xff;
        if (aux == 0x2b)
            // Counter-clockwise rotation event
            rotary->queue.enqueue(false);
        else if (aux == 0x17)
            // Clockwise rotation event
            rotary->queue.enqueue(true);
    }
}

// ----------------------------------------------------------------------------

/*
  DECODING ALGORITHM:
  This is a state machine.
  Let be "xy" the combined input from ENCODER_A (CLK) and ENCODER_B (DT).
  Each state is represented as four bits:
  - "1100": initial and final state
  - "0000": another initial and final state
  - "0001": transient state to clockwise rotation
  - "0010": transient state to counter-clockwise rotation
  - "1101": transient state to counter-clockwise rotation
  - "1110": transient state to clockwise rotation
*/

void RotaryEncoderInput::isrhAlternateEncoding(void *instance)
{
    // UBaseType_t lock = taskENTER_CRITICAL_FROM_ISR();
    UBaseType_t lock = portSET_INTERRUPT_MASK_FROM_ISR();
    RotaryEncoderInput *rotary = (RotaryEncoderInput *)instance;
    int clk = GPIO_GET_LEVEL(rotary->clkPin);
    int dt = GPIO_GET_LEVEL(rotary->dtPin);
    portCLEAR_INTERRUPT_MASK_FROM_ISR(lock);
    // taskEXIT_CRITICAL_FROM_ISR(lock);

    uint8_t reading = ((clk << 1) | dt);
    uint8_t initial_state = (rotary->code & 0b1100);
    if ((reading == 0b01) || (reading == 0b10))
    {
        rotary->code = initial_state | reading;
        return;
    }
    else // ((reading == 0b00) || (reading == 0b11))
    {
        uint8_t transient_state = (rotary->code & 0b0011);
        initial_state = initial_state >> 2;
        if (initial_state == reading)
            // bouncing (ignore)
            return;
        if (initial_state == 0b00)
        {
            if (transient_state == 0b01)
            {
                // Clockwise
                rotary->queue.enqueue(true);
                rotary->code = 0b1100;
            }
            else if (transient_state == 0b10)
            {
                // Counter-clockwise
                rotary->queue.enqueue(false);
                rotary->code = 0b1100;
            }
        }
        else // (initial_state==0b11)
        {
            if (transient_state == 0b01)
            {
                // Counter-clockwise
                rotary->queue.enqueue(false);
                rotary->code = 0b0000;
            }
            else if (transient_state == 0b10)
            {
                // Clockwise
                rotary->queue.enqueue(true);
                rotary->code = 0b0000;
            }
        }
    }
}

//-------------------------------------------------------------------

RotaryEncoderInput::RotaryEncoderInput(
    InputGPIO clkPin,
    InputGPIO dtPin,
    InputNumber cwButtonNumber,
    InputNumber ccwButtonNumber,
    bool useAlternateEncoding)
{
    // Initialize properties
    this->clkPin = clkPin;
    this->dtPin = dtPin;
    this->cwButton = cwButtonNumber;
    this->ccwButton = ccwButtonNumber;
    sequence = 0;
    pressEventNotified = false;
    currentPulseWidth = 0;

    // Config gpio
    internals::hal::gpio::forInput(clkPin, false, true);
    internals::hal::gpio::forInput(dtPin, false, true);

    // Initialize decoding state machine
    if (useAlternateEncoding)
    {
        // Determine the initial state in alternate encoding
        uint8_t retries = 0;
        do
        {
            // Loop until a non-transient state is detected
            // (code==0b0000) or (code==0b1100)
            int clk = GPIO_GET_LEVEL(clkPin);
            int dt = GPIO_GET_LEVEL(dtPin);
            code = ((clk << 1) | dt) << 2;
            retries++;
        } while ((code != 0b0000) && (code != 0b1100) && (retries < 10));
        if (retries >= 10)
            // The rotary encoder is stuck in a transient state.
            // The first rotation may produce a wrong event.
            code = 0b0000;
    }
    else
    {
        code = 0;
        isrh(this);
        isrh(this);
    }

    // Enable IRQ for dtPin
    if (useAlternateEncoding)
    {
        internals::hal::gpio::enableISR(
            dtPin, isrhAlternateEncoding, (void *)this);
        internals::hal::gpio::enableISR(
            clkPin, isrhAlternateEncoding, (void *)this);
    }
    else
    {
        internals::hal::gpio::enableISR(dtPin, isrh, (void *)this);
        internals::hal::gpio::enableISR(clkPin, isrh, (void *)this);
    }
}

// ----------------------------------------------------------------------------

void RotaryEncoderInput::read(uint128_t &state)
{
    if (currentPulseWidth > 0)
    {
        currentPulseWidth--;
        if ((currentPulseWidth == 0) && (pressEventNotified))
        {
            // Move from "on" phase to "off" phase
            pressEventNotified = false;
            currentPulseWidth = pulseMultiplier;
        }
        if (pressEventNotified)
        {
            // "On" phase in progress
            state.set_bit(cwButton, cwOrCcw);
            state.set_bit(ccwButton, !cwOrCcw);
        }
        else
        {
            // "Off" phase in progress
            state.set_bit(cwButton, false);
            state.set_bit(ccwButton, false);
        }
        return;
    }
    else
    {
        if (queue.dequeue(cwOrCcw))
        {
            // start a "pulse"
            pressEventNotified = true;
            currentPulseWidth = pulseMultiplier;
            state.set_bit(cwButton, cwOrCcw);
            state.set_bit(ccwButton, !cwOrCcw);
        }
        else
        {
            // No input event
            state.set_bit(cwButton, false);
            state.set_bit(ccwButton, false);
        }
    }
}

//-------------------------------------------------------------------
// Button matrix
//-------------------------------------------------------------------

ButtonMatrixInput::ButtonMatrixInput(
    const ButtonMatrix &matrix,
    bool negativeLogic)
{
    // copy parameters
    this->negativeLogic = negativeLogic;
    this->matrix = matrix;

    // Compute mask and initialize GPIO pins
    for (auto row : this->matrix)
    {
        internals::hal::gpio::forOutput(
            row.first,
            negativeLogic,
            negativeLogic);
        for (auto col : row.second)
        {
            internals::hal::gpio::forInput(
                col.first,
                !negativeLogic,
                negativeLogic);
        }
    }
}

//-------------------------------------------------------------------

void ButtonMatrixInput::read(uint128_t &state)
{
    for (auto row : matrix)
    {
        GPIO_SET_LEVEL(row.first, !negativeLogic);
        // Wait for the signal to change from LOW to HIGH
        // due to parasite capacitances.
        signal_change_delay(5);
        for (auto col : row.second)
        {
            int level = GPIO_GET_LEVEL((int)col.first);
            state.set_bit(col.second, (bool)(level ^ negativeLogic));
        }
        GPIO_SET_LEVEL(row.first, negativeLogic);
        // Wait for the signal to change from HIGH to LOW.
        // Otherwise, there will be a false reading at the next iteration.
        signal_change_delay(5);
    }
}

//-------------------------------------------------------------------
// Analog Multiplexers
//-------------------------------------------------------------------

/**
 * @brief Get a vector of input numbers for a group
 *        of analog multiplexer chips
 *
 * @tparam PinTags Pin tags
 * @param selectorCount Count of selector pins
 * @param chips Group of analog multiplexer chips
 * @return ::std::vector<uint8_t> Input numbers
 */
template <typename PinTags>
::std::vector<uint8_t> getInputNumbers(
    uint8_t selectorCount,
    const AnalogMultiplexerGroup<PinTags> &chips)
{
    ::std::vector<uint8_t> result{};
    uint8_t inputCount = chips.size();
    size_t arrayLength = (inputCount << selectorCount);
    for (size_t i = 0; i < arrayLength; i++)
        result.push_back(0xFF);

    // Populate the vector
    for (size_t chipIndex = 0; chipIndex < inputCount; chipIndex++)
    {
        const auto &chip = chips[chipIndex];
        for (auto map_pair : chip)
        {
            // Note:
            // map_pair.first = PinTags
            // map_pair.second = InputNumber
            uint8_t chipPinIndex = static_cast<uint8_t>(map_pair.first);
            // NOTE: switchIndex = (chipIndex * 2^selectors.size) + chipPinIndex
            uint8_t switchIndex = (chipIndex << selectorCount) + chipPinIndex;
            result[switchIndex] = (map_pair.second);
        }
    }
    return result;
}

//-------------------------------------------------------------------

/**
 * @brief Get the input pins in a group of
 *        analog multiplexer chips
 *
 * @tparam PinTags Pin tags
 * @param chips Group of analog multiplexer chips
 * @return InputGPIOCollection Input pins
 */
template <typename PinTags>
::std::vector<InputGPIO> getInputPins(
    const AnalogMultiplexerGroup<PinTags> &chips)
{
    ::std::vector<InputGPIO> inputs;
    for (auto chip : chips)
        inputs.push_back(chip.getInputGPIO());
    return inputs;
}

//-------------------------------------------------------------------

void AnalogMultiplexerInput::initializeMux(
    ::std::initializer_list<OutputGPIO> selectorPins,
    const ::std::vector<InputGPIO> &inputPins)
{
    // Initialize GPIO pins
    this->selectorPins = selectorPins;
    this->inputPins = inputPins;
    for (auto pin : this->selectorPins)
        internals::hal::gpio::forOutput(pin, false, false);
    for (auto pin : this->inputPins)
        internals::hal::gpio::forInput(pin, false, true);
}

//-------------------------------------------------------------------

AnalogMultiplexerInput ::AnalogMultiplexerInput(
    OutputGPIO selectorPin1,
    OutputGPIO selectorPin2,
    OutputGPIO selectorPin3,
    const AnalogMultiplexerGroup<Mux8Pin> &chips)
{
    initializeMux(
        {selectorPin1, selectorPin2, selectorPin3},
        getInputPins(chips));
    this->inputNumber = getInputNumbers(selectorPins.size(), chips);
}

//-------------------------------------------------------------------

AnalogMultiplexerInput::AnalogMultiplexerInput(
    OutputGPIO selectorPin1,
    OutputGPIO selectorPin2,
    OutputGPIO selectorPin3,
    OutputGPIO selectorPin4,
    const AnalogMultiplexerGroup<Mux16Pin> &chips)
{
    initializeMux(
        {selectorPin1, selectorPin2, selectorPin3, selectorPin4},
        getInputPins(chips));
    this->inputNumber = getInputNumbers(selectorPins.size(), chips);
}

//-------------------------------------------------------------------

AnalogMultiplexerInput::AnalogMultiplexerInput(
    OutputGPIO selectorPin1,
    OutputGPIO selectorPin2,
    OutputGPIO selectorPin3,
    OutputGPIO selectorPin4,
    OutputGPIO selectorPin5,
    const AnalogMultiplexerGroup<Mux32Pin> &chips)
{
    initializeMux(
        {selectorPin1, selectorPin2, selectorPin3, selectorPin4, selectorPin5},
        getInputPins(chips));
    this->inputNumber = getInputNumbers(selectorPins.size(), chips);
}

//-------------------------------------------------------------------

void AnalogMultiplexerInput::read(uint128_t &state)
{
    auto switchCount = inputNumber.size();
    for (uint8_t switchIndex = 0; switchIndex < switchCount; switchIndex++)
    {
        // Choose selector pins
        for (uint8_t selPinIndex = 0;
             selPinIndex < selectorPins.size();
             selPinIndex++)
            GPIO_SET_LEVEL(
                selectorPins[selPinIndex],
                switchIndex & (1 << selPinIndex));

        // Wait for the signal to propagate.
        //
        // This delay is a worst-case value.
        // According to the 74HC4067 datasheet,
        // "Sn to Out" delay is in the range [51,450] nanoseconds
        // depending on air temperature, Vcc voltage and wire capacitance.
        // This range is [48,340] nanoseconds in the 74HC4051.
        // During tests, an empirical value of 250 nanoseconds worked,
        // but not less.
        signal_change_delay(450);

        uint8_t inputPinIndex = switchIndex >> selectorPins.size();
        int level = GPIO_GET_LEVEL(inputPins[inputPinIndex]);
        // Negative logic
        state.set_bit(inputNumber[switchIndex], !level);
    };
}

//-------------------------------------------------------------------
// I2C input hardware
//-------------------------------------------------------------------

I2CInput::I2CInput(
    uint8_t address7Bits,
    I2CBus bus,
    uint8_t max_speed_mult)
{
    internals::hal::i2c::abortOnInvalidAddress(address7Bits);
    if (!internals::hal::i2c::probe(address7Bits, bus))
        throw i2c_device_not_found(address7Bits, (int)bus);
    device = static_cast<void *>(
        internals::hal::i2c::add_device(
            address7Bits,
            max_speed_mult,
            bus));
}

I2CInput::~I2CInput()
{
    internals::hal::i2c::remove_device(I2C_SLAVE(device));
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------

PCF8574ButtonsInput::PCF8574ButtonsInput(
    const PCF8574Expander &inputNumbers,
    uint8_t address7Bits,
    I2CBus bus)
    : I2CInput(address7Bits, bus, 1)
{
    this->inputNumbers = inputNumbers;

    // The PCF8574 does not have internal registers
    // Read GPIO registers in order to clear all interrupts
    uint8_t dummy;
    getGPIOstate(dummy);
}

//-------------------------------------------------------------------

bool PCF8574ButtonsInput::getGPIOstate(uint8_t &state)
{
    esp_err_t err = i2c_master_receive(
        I2C_SLAVE(device),
        (uint8_t *)&state,
        1,
        I2C_TIMEOUT_MS);
    if (err == ESP_OK)
        state = ~state; // convert to positive logic
    return (err == ESP_OK);
}

//-------------------------------------------------------------------

void PCF8574ButtonsInput::read(uint128_t &state)
{
    uint8_t GPIOstate;
    if (getGPIOstate(GPIOstate))
    {
        for (auto spec : inputNumbers)
        {
            // Note:
            // spec.first = pin number in the chip (0-7)
            // spec.second = input number
            state.set_bit(spec.second, GPIOstate & (1 << (int)spec.first));
        }
    }
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------

MCP23017ButtonsInput::MCP23017ButtonsInput(
    const MCP23017Expander &inputNumbers,
    uint8_t address7Bits,
    I2CBus bus) : I2CInput(address7Bits, bus, 1)
{
    this->inputNumbers = inputNumbers;
    configure();

    // Read GPIO registers in order to clear all interrupts
    uint16_t dummy;
    getGPIOstate(dummy);
}

//-------------------------------------------------------------------

void MCP23017ButtonsInput::configure()
{
    uint8_t buffer[3];

    // Configure IOCON register:
    // - Registers are in the same bank
    // - Interrupt pins mirrored
    // - Sequential operation
    // - Active driver output for interrupt pins
    // - Interrupt pins active low
    buffer[0] = MCP23017_IO_CONFIGURATION;
    buffer[1] = 0b01000000;
    ESP_ERROR_CHECK(
        i2c_master_transmit(I2C_SLAVE(device), buffer, 2, I2C_TIMEOUT_MS));

    // Set mode to "input"
    buffer[0] = MCP23017_IO_DIRECTION;
    buffer[1] = 0xFF;
    buffer[2] = 0xFF;
    ESP_ERROR_CHECK(
        i2c_master_transmit(I2C_SLAVE(device), buffer, 3, I2C_TIMEOUT_MS));

    // Enable pull-up resistors
    buffer[0] = MCP23017_PULL_UP_RESISTORS;
    // buffer[1] = 0xFF;
    // buffer[2] = 0xFF;
    ESP_ERROR_CHECK(
        i2c_master_transmit(I2C_SLAVE(device), buffer, 3, I2C_TIMEOUT_MS));

    // Automatically convert negative logic to positive logic
    buffer[0] = MCP23017_POLARITY;
    // buffer[1] = 0xFF;
    // buffer[2] = 0xFF;
    ESP_ERROR_CHECK(
        i2c_master_transmit(I2C_SLAVE(device), buffer, 3, I2C_TIMEOUT_MS));

    // Enable interrupts at all GPIO pins
    buffer[0] = MCP23017_INTERRUPT_ON_CHANGE;
    // buffer[1] = 0xFF;
    // buffer[2] = 0xFF;
    ESP_ERROR_CHECK(
        i2c_master_transmit(I2C_SLAVE(device), buffer, 3, I2C_TIMEOUT_MS));

    // Trigger interrupts by comparison with DEFVAL registers

    buffer[0] = MCP23017_INTERRUPT_CONTROL;
    // buffer[1] = 0xFF;
    // buffer[2] = 0xFF;
    ESP_ERROR_CHECK(
        i2c_master_transmit(I2C_SLAVE(device), buffer, 3, I2C_TIMEOUT_MS));

    // Set DEFVAL registers for interrupts
    buffer[0] = MCP23017_INTERRUPT_DEFAULT_VALUE;
    buffer[1] = 0; // Note: negative logic
    buffer[2] = 0;
    ESP_ERROR_CHECK(
        i2c_master_transmit(I2C_SLAVE(device), buffer, 3, I2C_TIMEOUT_MS));
}

//-------------------------------------------------------------------

bool MCP23017ButtonsInput::getGPIOstate(uint16_t &state)
{
    state = 0;
    static uint8_t cmd = MCP23017_GPIO;
    esp_err_t err = i2c_master_transmit_receive(
        I2C_SLAVE(device),
        &cmd,
        1,
        (uint8_t *)&state,
        2,
        I2C_TIMEOUT_MS);
    return (err == ESP_OK);
}

//-------------------------------------------------------------------

void MCP23017ButtonsInput::read(uint128_t &state)
{
    uint16_t GPIOstate;
    if (getGPIOstate(GPIOstate))
    {
        for (auto spec : inputNumbers)
        {
            // Note:
            // spec.first = pin number in the chip (0-15)
            // spec.second = input number
            state.set_bit(spec.second, GPIOstate & (1 << (int)spec.first));
        }
    }
}

//-------------------------------------------------------------------
// Shift registers
//-------------------------------------------------------------------

ShiftRegistersInput::ShiftRegistersInput(
    OutputGPIO loadPin,
    OutputGPIO nextPin,
    InputGPIO inputPin,
    const ShiftRegisterChain &chain,
    InputNumber SER_inputNumber,
    const bool loadHighOrLow,
    const bool nextHighToLowOrLowToHigh,
    const bool negativeLogic)
{
    // Initialize fields
    this->serialPin = inputPin;
    this->loadPin = loadPin;
    this->nextPin = nextPin;
    this->loadHighOrLow = loadHighOrLow;
    this->nextHighToLowOrLowToHigh = nextHighToLowOrLowToHigh;
    this->negativeLogic = negativeLogic;

    // Compute the count of switches in the chain
    switchCount = (8 * chain.size());
    if (SER_inputNumber != UNSPECIFIED::VALUE)
        switchCount++;

    // Initialize the vector of input numbers
    for (size_t i = 0; i < switchCount; i++)
        inputNumber.push_back(0xFF);

    // Compute the contents of the vector
    for (size_t chipIndex = 0; chipIndex < chain.size(); chipIndex++)
    {
        auto &chip = chain[chipIndex];
        for (auto map_pair : chip)
        {
            // Note:
            // map_pair.first = SR8Pin
            // map_pair.second = input number
            size_t arrayIndex =
                (chipIndex * 8) +
                static_cast<uint8_t>(map_pair.first);
            inputNumber[arrayIndex] = static_cast<uint8_t>(map_pair.second);
        }
    }
    if (SER_inputNumber != UNSPECIFIED::VALUE)
        inputNumber[switchCount - 1] = SER_inputNumber;

    // Initialize pins
    internals::hal::gpio::forOutput(loadPin, !loadHighOrLow, false);
    internals::hal::gpio::forOutput(nextPin, nextHighToLowOrLowToHigh, false);
    internals::hal::gpio::forInput(inputPin, false, false);
}

//-------------------------------------------------------------------

void ShiftRegistersInput::read(uint128_t &state)
{
    // Parallel load
    GPIO_SET_LEVEL(loadPin, loadHighOrLow);
    signal_change_delay(45);
    GPIO_SET_LEVEL(loadPin, !loadHighOrLow);

    // Serial output
    for (size_t switchIndex = 0; switchIndex < switchCount; switchIndex++)
    {
        int level = GPIO_GET_LEVEL(serialPin);
        state.set_bit(inputNumber[switchIndex], (level ^ negativeLogic));

        // next
        GPIO_SET_LEVEL(nextPin, !nextHighToLowOrLowToHigh);
        signal_change_delay(45);
        GPIO_SET_LEVEL(nextPin, nextHighToLowOrLowToHigh);
    }
}

//-------------------------------------------------------------------
// Analog clutch paddle
//-------------------------------------------------------------------

AnalogClutchInput::AnalogClutchInput(ADC_GPIO pinNumber)
{
    // Initialize
    this->pinNumber = pinNumber;
    lastADCReading = 0;

    // Note: we assume the potentiometer works on the full range of voltage.
    // If that is not the case, the user should ask for recalibration.
    // Storage of calibration data is handled at `Inputs.cpp`
    minADCReading = 0;
    maxADCReading = 254;
}

//-------------------------------------------------------------------

void AnalogClutchInput::read(uint8_t &value, bool &autocalibrated)
{
    // read ADC and remove 4 bits of noise
    int currentReading = internals::hal::gpio::getADCreading(pinNumber) >> 4;
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
    else
        value = map_value(
            currentReading,
            minADCReading,
            maxADCReading,
            CLUTCH_FULL_VALUE,
            CLUTCH_NONE_VALUE);
    lastADCReading = currentReading;
}

void AnalogClutchInput::getCalibrationData(int &minReading, int &maxReading)
{
    minReading = this->minADCReading;
    maxReading = this->maxADCReading;
}

void AnalogClutchInput::resetCalibrationData()
{
    minADCReading = INT_MAX;
    maxADCReading = INT_MIN;
}

void AnalogClutchInput::setCalibrationData(int minReading, int maxReading)
{
    minADCReading = minReading;
    maxADCReading = maxReading;
}