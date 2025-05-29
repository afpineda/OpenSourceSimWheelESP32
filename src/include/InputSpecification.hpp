/**
 * @file InputSpecification.hpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-02
 * @brief Configure input hardware and specify input numbers
 *
 * @note All input numbers are initialized to UNSPECIFIED::VALUE.
 *
 * @copyright Licensed under the EUPL
 *
 */

#pragma once

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "SimWheelTypes.hpp"
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <map>
#include <cstring> // For memset()

//-------------------------------------------------------------------
// Input specification: Button matrix
//-------------------------------------------------------------------

/**
 * @brief Button matrix specification
 *
 */
typedef std::map<OutputGPIO, std::map<InputGPIO, InputNumber>> ButtonMatrix;

/**
 * @brief Populate a button matrix with sequential input numbers
 *
 * @param matrix Button matrix
 * @param selectors Collection of selector pins
 * @param inputs Collection of input pins
 * @param first First input number to be assigned
 */
inline void populateButtonMatrix(
    ButtonMatrix &matrix,
    const OutputGPIOCollection selectors,
    const InputGPIOCollection inputs,
    const InputNumber first)
{
    uint8_t in = (uint8_t)first;
    for (auto row : selectors)
        for (auto col : inputs)
            matrix[row][col] = in++;
}

//-------------------------------------------------------------------
// Input specification: analog multiplexers
//-------------------------------------------------------------------

/**
 * @brief 74HC4051N pin tags for switches
 *        or any other 8-channel multiplexer
 *
 */
enum class Mux8Pin
{
    A0 = 0,
    A1,
    A2,
    A3,
    A4,
    A5,
    A6,
    A7
};

/**
 * @brief CD74HCx4067 pin tags for switches
 *        or any other 16-channel multiplexer
 *
 */
enum class Mux16Pin
{
    I0 = 0,
    I1,
    I2,
    I3,
    I4,
    I5,
    I6,
    I7,
    I8,
    I9,
    I10,
    I11,
    I12,
    I13,
    I14,
    I15,
};

/**
 * @brief ADG732 pin tags for switches
 *        or any other 32-channel multiplexer
 *
 */
enum class Mux32Pin
{
    S1 = 0,
    S2,
    S3,
    S4,
    S5,
    S6,
    S7,
    S8,
    S9,
    S10,
    S11,
    S12,
    S13,
    S14,
    S15,
    S16,
    S17,
    S18,
    S19,
    S20,
    S21,
    S22,
    S23,
    S24,
    S25,
    S26,
    S27,
    S28,
    S29,
    S30,
    S31,
    S32
};

/**
 * @brief Generic analog multiplexer chip
 *
 * @tparam PinTags Chip's pin tags
 */
template <typename PinTags>
struct AnalogMultiplexerChip : public std::map<PinTags, InputNumber>
{
public:
    /**
     * @brief Construct a new Analog Multiplexer Chip object
     *
     * @param inputPin Input GPIO attached to the multiplexed signal
     */
    AnalogMultiplexerChip(InputGPIO inputPin)
        : std::map<PinTags, InputNumber>(), inputGPIO{inputPin} {}

    /**
     * @brief Reserve the input pin and book all input numbers
     *
     * @note No need to use in user code
     */
    void reserve_and_book()
    {
        inputGPIO.reserve();
        for (auto i = this->begin(); i != this->end(); i++)
            i->second.book();
    }

    /**
     * @brief Get the input GPIO pin
     *
     * @return InputGPIO Input GPIO attached to the multiplexed signal
     */
    inline InputGPIO getInputGPIO() { return inputGPIO; }

private:
    InputGPIO inputGPIO;
};

/**
 * @brief 8-channel analog multiplexer chip
 *
 */
typedef AnalogMultiplexerChip<Mux8Pin> AnalogMultiplexerChip8;

/**
 * @brief 16-channel analog multiplexer chip
 *
 */
typedef AnalogMultiplexerChip<Mux16Pin> AnalogMultiplexerChip16;

/**
 * @brief 32-channel analog multiplexer chip
 *
 */
typedef AnalogMultiplexerChip<Mux32Pin> AnalogMultiplexerChip32;

/**
 * @brief Group of analog multiplexer chips sharing the same selector pins
 *
 * @tparam PinTags Pin tags
 */
template <typename PinTags>
using AnalogMultiplexerGroup = std::vector<AnalogMultiplexerChip<PinTags>>;

//-------------------------------------------------------------------
// Input specification: GPIO Expanders
//-------------------------------------------------------------------

/**
 * @brief MCP23017 pin tags for switches
 *
 */
enum class MCP23017Pin
{
    GPA0 = 0,
    GPA1,
    GPA2,
    GPA3,
    GPA4,
    GPA5,
    GPA6,
    GPA7,
    GPB0,
    GPB1,
    GPB2,
    GPB3,
    GPB4,
    GPB5,
    GPB6,
    GPB7
};

/**
 * @brief MCP23017 pin tags for switches
 *
 */
enum class PCF8574Pin
{
    P0 = 0,
    P1,
    P2,
    P3,
    P4,
    P5,
    P6,
    P7
};

/**
 * @brief Generic GPIO expander chip
 *
 * @tparam PinTags
 */
template <typename PinTags>
using GPIOExpanderChip = std::map<PinTags, InputNumber>;

/**
 * @brief MCP23017 GPIO Expander for switches
 *
 */
typedef GPIOExpanderChip<MCP23017Pin> MCP23017Expander;

/**
 * @brief PCF8574 GPIO Expander for switches
 *
 */
typedef GPIOExpanderChip<PCF8574Pin> PCF8574Expander;

//-------------------------------------------------------------------
// Input specification: PISO Shift registers
//-------------------------------------------------------------------

/**
 * @brief 74HC165N pin tags for switches
 *
 */
enum class SR8Pin
{
    H = 0,
    G,
    F,
    E,
    D,
    C,
    B,
    A
};

/**
 * @brief PISO shift register chip
 *
 */
typedef std::map<SR8Pin, InputNumber> ShiftRegisterChip;

/**
 * @brief Chain of PISO shift registers for switches
 *
 * @note The left-most chip is the first in the chain,
 *       attached to the input pin at the DevKit board.
 *       The right-most chip is the last in the chain.
 *       This one can have another switch attached to
 *       the SER pin.
 */
typedef std::vector<ShiftRegisterChip> ShiftRegisterChain;

//-------------------------------------------------------------------
// Input specification: coded rotary switch
//-------------------------------------------------------------------

/**
 * @brief Coded rotary switch
 *
 */
typedef std::map<uint8_t, InputNumber> RotaryCodedSwitch;
