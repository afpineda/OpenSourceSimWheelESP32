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
#include "driver/i2c.h"    // For I2C operation
#include "esp32-hal.h"     // For portSET_INTERRUPT_MASK_FROM_ISR
#include "driver/gpio.h"   // For gpio_set_level/gpio_get_level()
#include "esp32-hal-cpu.h" // For getCpuFrequencyMhz()

//-------------------------------------------------------------------
// Globals
//-------------------------------------------------------------------

// I2C
#define I2C_TIMEOUT_TICKS pdMS_TO_TICKS(30)

// MCP23017 registers
#define MCP23017_IO_CONFIGURATION 0x0A
#define MCP23017_IO_DIRECTION 0x00
#define MCP23017_PULL_UP_RESISTORS 0x0C
#define MCP23017_GPIO 0x12
#define MCP23017_POLARITY 0x02
#define MCP23017_INTERRUPT_ON_CHANGE 0x04
#define MCP23017_INTERRUPT_CONTROL 0x08
#define MCP23017_INTERRUPT_DEFAULT_VALUE 0x06

#pragma optimize("", off)
inline void signal_change_delay(uint32_t nanoseconds)
{
    // Note: 1 ns = 1000 MHz
    static uint32_t instructionTimeNs = (getCpuFrequencyMhz() < 1000) ? (1000 / getCpuFrequencyMhz()) : 1;
    for (uint32_t delay = 0; delay < nanoseconds; delay += instructionTimeNs)
        __asm__ __volatile__(" nop\n");
}
#pragma optimize("", on)

//-------------------------------------------------------------------
// Single button
//-------------------------------------------------------------------

DigitalButton::DigitalButton(InputGPIO pinNumber, InputNumber buttonNumber) : DigitalInput()
{
    internals::hal::gpio::forInput(pinNumber, false, true);
    this->pinNumber = pinNumber;
    this->bitmap = (uint64_t)buttonNumber;
    this->mask = ~(this->bitmap);
}

//-------------------------------------------------------------------

uint64_t DigitalButton::read(uint64_t lastState)
{
    int reading = GPIO_GET_LEVEL(pinNumber);
    if (reading)
        // Pulled-up input
        return 0ULL;
    else
        return bitmap;
}

//-------------------------------------------------------------------
// Rotary encoder
//-------------------------------------------------------------------

// Note: IRAM_ATTR no longer needed
void RotaryEncoderInput::isrh(void *instance)
{
    static const uint8_t valid_code[] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

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
// state = xyXY
// where xy = previous input, XY = current input, x or X=CLK, y or Y=DT
//
// States and transitions (mermaid graph):
// graph TD
//  0011 --01--> 1101 --00--> 0100 --CCW--> 0000
//  0000 --10--> 0010 --11--> 1011 --CCW--> 0011
//  0011 --10--> 1110 --00--> 1000 --CW--> 0000
//  0000 --01--> 0001 --11--> 0111 --CW--> 0011
//
// transition = aaaabbbb
// where aaaa=previous state, bbbb=current state
//
// Valid transitions:
// 00111101
// 11010100 (CCW to 0000)
// 00000010
// 00101011 (CCW to 0011)
// 00111110
// 11101000 (CW to 0000)
// 00000001
// 00010111 (CW to 0011)

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
    uint8_t nextCode = ((rotary->code << 2) | reading) & 0b1111;
    uint8_t transition = (rotary->code << 4) | nextCode;

    if ((transition == 0b00111101) ||
        (transition == 0b11010100) ||
        (transition == 0b00000010) ||
        (transition == 0b00101011) ||
        (transition == 0b00111110) ||
        (transition == 0b11101000) ||
        (transition == 0b00000001) ||
        (transition == 0b00010111))
    {
        if (transition == 0b11010100)
        {
            // Clockwise rotation event
            rotary->code = 0;
            rotary->queue.enqueue(true);
        }
        else if (transition == 0b00101011)
        {
            // Clockwise rotation event
            rotary->code = 0b11;
            rotary->queue.enqueue(true);
        }
        else if (transition == 0b11101000)
        {
            // Counter-clockwise rotation event
            rotary->code = 0;
            rotary->queue.enqueue(false);
        }
        else if (transition == 0b00010111)
        {
            // Counter-clockwise rotation event
            rotary->code = 0b11;
            rotary->queue.enqueue(false);
        }
        else
            rotary->code = nextCode;
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
    this->cwButtonBitmap = (uint64_t)cwButtonNumber;
    this->ccwButtonBitmap = (uint64_t)ccwButtonNumber;
    mask = ~((this->cwButtonBitmap) | (this->ccwButtonBitmap));
    sequence = 0;
    pressEventNotified = false;
    currentPulseWidth = 0;

    // Config gpio
    internals::hal::gpio::forInput(clkPin, false, true);
    internals::hal::gpio::forInput(dtPin, false, true);

    // Initialize decoding state machine
    if (useAlternateEncoding)
    {
        code = 0b11;
        isrhAlternateEncoding(this);
        isrhAlternateEncoding(this);
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
        internals::hal::gpio::enableISR(dtPin, isrhAlternateEncoding, (void *)this);
        internals::hal::gpio::enableISR(clkPin, isrhAlternateEncoding, (void *)this);
    }
    else
    {
        internals::hal::gpio::enableISR(dtPin, isrh, (void *)this);
        internals::hal::gpio::enableISR(clkPin, isrh, (void *)this);
    }
}

// ----------------------------------------------------------------------------

uint64_t RotaryEncoderInput::read(uint64_t lastState)
{
    if (currentPulseWidth > 0)
    {
        currentPulseWidth--;
        if (currentPulseWidth == 0)
        {
            if (pressEventNotified)
            {
                pressEventNotified = false;
                currentPulseWidth = pulseMultiplier;
            }
            return 0ULL;
        }
        // "pulse" in progress
        return lastState & ~mask;
    }
    else
    {
        bool cwOrCcw;
        if (queue.dequeue(cwOrCcw))
        {
            // start a "pulse"
            pressEventNotified = true;
            currentPulseWidth = pulseMultiplier;
            if (cwOrCcw)
                return cwButtonBitmap;
            else
                return ccwButtonBitmap;
        }
        else
            // No input event
            return 0ULL;
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
        internals::hal::gpio::forOutput(row.first, negativeLogic, false);
        for (auto col : row.second)
        {
            internals::hal::gpio::forInput(col.first, !negativeLogic, negativeLogic);
            addToMask((uint64_t)col.second);
        }
    }
}

//-------------------------------------------------------------------

uint64_t ButtonMatrixInput::read(uint64_t lastState)
{
    uint64_t state = 0ULL;
    for (auto row : matrix)
    {
        GPIO_SET_LEVEL(row.first, !negativeLogic);
        // Wait for the signal to change from LOW to HIGH due to parasite capacitances.
        signal_change_delay(5);
        for (auto col : row.second)
        {
            int level = GPIO_GET_LEVEL((int)col.first);
            if (level ^ negativeLogic)
                state |= (uint64_t)col.second;
        }
        GPIO_SET_LEVEL(row.first, negativeLogic);
        // Wait for the signal to change from HIGH to LOW.
        // Otherwise, there will be a false reading at the next iteration.
        signal_change_delay(5);
    }
    return state;
}

//-------------------------------------------------------------------
// Analog Multiplexers
//-------------------------------------------------------------------

/**
 * @brief Create a bitmap array for a group
 *        of analog multiplexer chips
 *
 * @tparam PinTags Pin tags
 * @param selectorCount Count of selector pins
 * @param chips Group of analog multiplexer chips
 * @return uint64_t* A bitmap array
 */
template <typename PinTags>
uint64_t *createBitmap(
    uint8_t selectorCount,
    const AnalogMultiplexerGroup<PinTags> &chips)
{
    uint8_t inputCount = chips.size();

    // Create bidimensional array for input bitmaps
    size_t arrayLength = (inputCount << selectorCount);
    uint64_t *bitmap = new uint64_t[arrayLength];
    std::memset(bitmap, 0, arrayLength * sizeof(uint64_t));

    // Populate the array of input bitmaps
    for (size_t chipIndex = 0; chipIndex < inputCount; chipIndex++)
    {
        const auto &chip = chips[chipIndex];
        for (auto map_pair : chip)
        {
            uint8_t chipPinIndex = static_cast<uint8_t>(map_pair.first); // first is PinTags
            // NOTE: switchIndex = (chipIndex * 2^selectors.size) + chipPinIndex
            uint8_t switchIndex = (chipIndex << selectorCount) + chipPinIndex;
            bitmap[switchIndex] = (uint64_t)(map_pair.second); // second is InputNumber
        }
    }
    return bitmap;
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
InputGPIOCollection getInputPins(const AnalogMultiplexerGroup<PinTags> &chips)
{
    InputGPIOCollection inputs;
    for (auto chip : chips)
        inputs.push_back(chip.getInputGPIO());
    return inputs;
}

//-------------------------------------------------------------------

void AnalogMultiplexerInput::initializeMux()
{
    // Initialize GPIO pins
    for (auto pin : selectorPins)
        internals::hal::gpio::forOutput(pin, false, false);
    for (auto pin : inputPins)
        internals::hal::gpio::forInput(pin, false, true);

    // Compute mask
    // NOTE: switchCount = (2^selectorPins.size) * inputPins.size;
    switchCount = (inputPins.size() << selectorPins.size());
    for (size_t i = 0; i < switchCount; i++)
        addToMask(bitmap[i]);
}

//-------------------------------------------------------------------

AnalogMultiplexerInput ::AnalogMultiplexerInput(
    OutputGPIO selectorPin1,
    OutputGPIO selectorPin2,
    OutputGPIO selectorPin3,
    const AnalogMultiplexerGroup<Mux8Pin> &chips)
{
    this->selectorPins = {selectorPin1, selectorPin2, selectorPin3};
    this->inputPins = getInputPins(chips);
    this->bitmap = createBitmap(selectorPins.size(), chips);
    initializeMux();
}

//-------------------------------------------------------------------

AnalogMultiplexerInput::AnalogMultiplexerInput(
    OutputGPIO selectorPin1,
    OutputGPIO selectorPin2,
    OutputGPIO selectorPin3,
    OutputGPIO selectorPin4,
    const AnalogMultiplexerGroup<Mux16Pin> &chips)
{
    this->selectorPins =
        {selectorPin1, selectorPin2, selectorPin3, selectorPin4};
    this->inputPins = getInputPins(chips);
    this->bitmap = createBitmap(selectorPins.size(), chips);
    initializeMux();
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
    this->selectorPins =
        {selectorPin1, selectorPin2, selectorPin3, selectorPin4, selectorPin5};
    this->inputPins = getInputPins(chips);
    this->bitmap = createBitmap(selectorPins.size(), chips);
    initializeMux();
}

//-------------------------------------------------------------------

uint64_t AnalogMultiplexerInput::read(uint64_t lastState)
{
    uint64_t state = 0ULL;
    for (uint8_t switchIndex = 0; switchIndex < switchCount; switchIndex++)
    {
        // Choose selector pins
        for (uint8_t selPinIndex = 0; selPinIndex < selectorPins.size(); selPinIndex++)
            GPIO_SET_LEVEL(selectorPins[selPinIndex], switchIndex & (1 << selPinIndex));

        // Wait for the signal to propagate
        signal_change_delay(25);

        uint8_t inputPinIndex = switchIndex >> selectorPins.size();
        int level = GPIO_GET_LEVEL(inputPins[inputPinIndex]);
        if (!level)
            // Negative logic
            state = state | bitmap[switchIndex];
    };
    return state;
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
    this->deviceAddress = (address7Bits << 1);
    this->bus = bus;
    internals::hal::i2c::require(max_speed_mult, bus);
    if (!internals::hal::i2c::probe(address7Bits, bus))
        throw i2c_device_not_found(address7Bits, (int)bus);
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

    // Compute mask
    for (auto spec : inputNumbers)
        addToMask((uint64_t)spec.second);

    // The PCF8574 does not have internal registers
    // Read GPIO registers in order to clear all interrupts
    uint64_t dummy;
    getGPIOstate(dummy);
}

//-------------------------------------------------------------------

bool PCF8574ButtonsInput::getGPIOstate(uint64_t &state)
{
    state = 0ULL;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, deviceAddress | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, (unsigned char *)&state, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    bool result = (i2c_master_cmd_begin(AS_PORT(bus), cmd, I2C_TIMEOUT_TICKS) == ESP_OK);
    i2c_cmd_link_delete(cmd);
    state = ~state; // convert to positive logic
    return result;
}
//-------------------------------------------------------------------

uint64_t PCF8574ButtonsInput::read(uint64_t lastState)
{
    uint64_t GPIOstate;
    if (getGPIOstate(GPIOstate))
    {
        uint64_t result = 0ULL;
        for (auto spec : inputNumbers)
        {
            if (GPIOstate & (1ULL << (int)spec.first))
                result |= (uint64_t)spec.second;
        }
        return result;
    }
    return lastState & ~mask;
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------

MCP23017ButtonsInput::MCP23017ButtonsInput(
    const MCP23017Expander &inputNumbers,
    uint8_t address7Bits,
    I2CBus bus) : I2CInput(address7Bits, bus, 1)
{
    this->inputNumbers = inputNumbers;

    // Compute mask
    for (auto spec : inputNumbers)
        addToMask((uint64_t)spec.second);

    configure();

    // Read GPIO registers in order to clear all interrupts
    uint64_t dummy;
    getGPIOstate(dummy);
}

//-------------------------------------------------------------------

void MCP23017ButtonsInput::configure()
{
    i2c_cmd_handle_t cmd;

    // Configure IOCON register:
    // - Registers are in the same bank
    // - Interrupt pins mirrored
    // - Sequential operation
    // - Active driver output for interrupt pins
    // - Interrupt pins active low
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, deviceAddress, true);
    i2c_master_write_byte(cmd, MCP23017_IO_CONFIGURATION, true);
    i2c_master_write_byte(cmd, 0b01000000, true);
    i2c_master_stop(cmd);
    ESP_ERROR_CHECK(i2c_master_cmd_begin(AS_PORT(bus), cmd, I2C_TIMEOUT_TICKS));
    i2c_cmd_link_delete(cmd);

    // Set mode to "input"
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, deviceAddress, true);
    i2c_master_write_byte(cmd, MCP23017_IO_DIRECTION, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_stop(cmd);
    ESP_ERROR_CHECK(i2c_master_cmd_begin(AS_PORT(bus), cmd, I2C_TIMEOUT_TICKS));
    i2c_cmd_link_delete(cmd);

    // Enable pull-up resistors
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, deviceAddress, true);
    i2c_master_write_byte(cmd, MCP23017_PULL_UP_RESISTORS, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_stop(cmd);
    ESP_ERROR_CHECK(i2c_master_cmd_begin(AS_PORT(bus), cmd, I2C_TIMEOUT_TICKS));
    i2c_cmd_link_delete(cmd);

    // Automatically convert negative logic to positive logic
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, deviceAddress, true);
    i2c_master_write_byte(cmd, MCP23017_POLARITY, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_stop(cmd);
    ESP_ERROR_CHECK(i2c_master_cmd_begin(AS_PORT(bus), cmd, I2C_TIMEOUT_TICKS));
    i2c_cmd_link_delete(cmd);

    // Enable interrupts at all GPIO pins
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, deviceAddress, true);
    i2c_master_write_byte(cmd, MCP23017_INTERRUPT_ON_CHANGE, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_stop(cmd);
    ESP_ERROR_CHECK(i2c_master_cmd_begin(AS_PORT(bus), cmd, I2C_TIMEOUT_TICKS));
    i2c_cmd_link_delete(cmd);

    // Trigger interrupts by comparison with DEFVAL registers
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, deviceAddress, true);
    i2c_master_write_byte(cmd, MCP23017_INTERRUPT_CONTROL, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_stop(cmd);
    ESP_ERROR_CHECK(i2c_master_cmd_begin(AS_PORT(bus), cmd, I2C_TIMEOUT_TICKS));
    i2c_cmd_link_delete(cmd);

    // Set DEFVAL registers for interrupts
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, deviceAddress, true);
    i2c_master_write_byte(cmd, MCP23017_INTERRUPT_DEFAULT_VALUE, true);
    i2c_master_write_byte(cmd, 0, true); // Note: negative logic
    i2c_master_write_byte(cmd, 0, true);
    i2c_master_stop(cmd);
    ESP_ERROR_CHECK(i2c_master_cmd_begin(AS_PORT(bus), cmd, I2C_TIMEOUT_TICKS));
    i2c_cmd_link_delete(cmd);
}

//-------------------------------------------------------------------

bool MCP23017ButtonsInput::getGPIOstate(uint64_t &state)
{
    state = 0ULL;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, deviceAddress, true);
    i2c_master_write_byte(cmd, MCP23017_GPIO, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, deviceAddress | I2C_MASTER_READ, true);
    i2c_master_read(cmd, (uint8_t *)&state, 2, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    bool result = (i2c_master_cmd_begin(AS_PORT(bus), cmd, I2C_TIMEOUT_TICKS) == ESP_OK);
    i2c_cmd_link_delete(cmd);
    return result;
}

//-------------------------------------------------------------------

uint64_t MCP23017ButtonsInput::read(uint64_t lastState)
{
    uint64_t GPIOstate;
    if (getGPIOstate(GPIOstate))
    {
        uint64_t result = 0ULL;
        for (auto spec : inputNumbers)
        {
            if (GPIOstate & (1ULL << (int)spec.first))
                result |= (uint64_t)spec.second;
        }
        return result;
    }
    return lastState & ~mask;
}

//-------------------------------------------------------------------
// Shift registers
//-------------------------------------------------------------------

/**
 * @brief Create a bitmap array for a chain
 *        of PISO shift registers
 *
 * @param[in] chain Chain of shift registers
 * @param[in] SER_inputNumber Input number assigned to SER in the last chip
 * @param[out] switchCount Count of switches (or array length)
 * @return uint64_t* Bitmap
 */
uint64_t *createBitmap(
    const ShiftRegisterChain &chain,
    InputNumber SER_inputNumber,
    size_t &switchCount)
{
    // Compute switch count
    switchCount = (8 * chain.size());
    if (SER_inputNumber != UNSPECIFIED::VALUE)
        switchCount++;

    // Reserve memory for all input bitmaps
    uint64_t *bitmap = new uint64_t[switchCount];
    std::memset(bitmap, 0, switchCount * sizeof(uint64_t));

    // Populate input bitmap array
    for (size_t chipIndex = 0; chipIndex < chain.size(); chipIndex++)
    {
        auto &chip = chain[chipIndex];
        for (auto map_pair : chip)
        {
            uint8_t chipPinIndex = static_cast<uint8_t>(map_pair.first); // first is SR8Pin
            size_t arrayIndex = (chipIndex * 8) + static_cast<uint8_t>(chipPinIndex);
            bitmap[arrayIndex] = (uint64_t)(map_pair.second); // second is InputNumber
        }
    }
    if (SER_inputNumber != UNSPECIFIED::VALUE)
        bitmap[switchCount - 1] = (uint64_t)SER_inputNumber;
    return bitmap;
}

//-------------------------------------------------------------------

ShiftRegistersInput::ShiftRegistersInput(
    OutputGPIO loadPin,
    OutputGPIO nextPin,
    InputGPIO inputPin,
    const ShiftRegisterChain &chain,
    InputNumber SER_inputNumber,
    const bool loadHighOrLow,
    const bool nextHighToLowOrLowToHigh)
{
    this->serialPin = inputPin;
    this->loadPin = loadPin;
    this->nextPin = nextPin;
    this->loadHighOrLow = loadHighOrLow;
    this->nextHighToLowOrLowToHigh = nextHighToLowOrLowToHigh;
    this->bitmap = createBitmap(chain, SER_inputNumber, switchCount);
    for (size_t i = 0; i < switchCount; i++)
        addToMask(bitmap[i]);

    // Initialize pins
    internals::hal::gpio::forOutput(loadPin, !loadHighOrLow, false);
    internals::hal::gpio::forOutput(nextPin, nextHighToLowOrLowToHigh, false);
    internals::hal::gpio::forInput(inputPin, false, false);
}

//-------------------------------------------------------------------

uint64_t ShiftRegistersInput::read(uint64_t lastState)
{
    uint64_t state = 0ULL;

    // Parallel load
    GPIO_SET_LEVEL(loadPin, loadHighOrLow);
    signal_change_delay(35);
    GPIO_SET_LEVEL(loadPin, !loadHighOrLow);

    // Serial output
    for (size_t switchIndex = 0; switchIndex < switchCount; switchIndex++)
    {
        int level = GPIO_GET_LEVEL(serialPin);
        if (!level)
            // negative logic
            state = state | bitmap[switchIndex];

        // next
        GPIO_SET_LEVEL(nextPin, !nextHighToLowOrLowToHigh);
        signal_change_delay(35);
        GPIO_SET_LEVEL(nextPin, nextHighToLowOrLowToHigh);
    }
    return state;
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
        value = map_value(currentReading, minADCReading, maxADCReading, CLUTCH_FULL_VALUE, CLUTCH_NONE_VALUE);
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