/**
 * @file InputHardware.hpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-02
 * @brief Input hardware classes
 *
 * @copyright Licensed under the EUPL
 *
 */

#pragma once

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "InternalTypes.hpp" // For BitQueue
#include "InputSpecification.hpp"

//-------------------------------------------------------------------
// (Abstract) DigitalInput
//-------------------------------------------------------------------

/**
 * @brief Base class for all polled switches
 *
 * @note Instances of this base class are supposed to
 *       live forever.
 */
class DigitalInput
{
public:
    /**
     * @brief Input mask. For read-only.
     *
     */
    uint64_t mask = ~0ULL;

public:
    virtual ~DigitalInput() noexcept {}

    /**
     * @brief Read the current state of the inputs (pressed or released)
     *
     * @warning This function must not set any bit outside of the
     *          input bitmask in the return value.
     *
     * @param lastState State of the same inputs as recorded in the previous iteration.
     *                  Whether the current state is unknown,
     *                  @p lastState must be returned (properly masked).
     *                  Must be set to zero at first call.
     * @return uint64_t Current state of the inputs
     *                  (a bit set to 1 means a pressed button).
     */
    virtual uint64_t read(uint64_t lastState) = 0;

protected:
    /**
     * @brief Add an input bitmap to the current mask
     *
     * @param bitmap Input bitmap
     */
    void addToMask(uint64_t bitmap) { mask &= ~bitmap; }
};

//-------------------------------------------------------------------
// Single button
//-------------------------------------------------------------------

/**
 * @brief Single button
 *
 */
class DigitalButton : public DigitalInput
{
protected:
    /// @brief Configured input pin
    InputGPIO pinNumber;
    /// @brief Input bitmap
    uint64_t bitmap;

public:
    /**
     * @brief Construct a new Digital Button object
     *
     * @param[in] pinNumber GPIO pin where the button is attached to
     * @param[in] buttonNumber Assigned number for this button
     */
    DigitalButton(InputGPIO pinNumber, InputNumber buttonNumber);

    virtual uint64_t read(uint64_t lastState) override;
};

//-------------------------------------------------------------------
// Rotary encoder
//-------------------------------------------------------------------

/**
 * @brief Relative Rotary Encoder
 *
 */
class RotaryEncoderInput : public DigitalInput
{
private:
    InputGPIO clkPin, dtPin; // pins
    uint8_t code;            // State of the decoding algorithm
    uint16_t sequence;       // Last sequence of states in "alternate encoding"
    uint64_t cwButtonBitmap;
    uint64_t ccwButtonBitmap;
    BitQueue queue;

    // duration of the current "pulse" event in polling cycles
    uint8_t currentPulseWidth;

    // a "virtual button" press event was notified at read(),
    // so a release event must be notified next
    bool pressEventNotified;

    static void isrh(void *instance);
    static void isrhAlternateEncoding(void *instance);

public:
    /**
     * @brief Pulse multiplier for rotary encoders
     *
     * @note For read only. Do not overwrite.
     *       Always greater than zero.
     */
    inline static uint8_t pulseMultiplier = 1;

    /**
     * @brief Construct a new Rotary Encoder Input object
     *
     * @param[in] clkPin GPIO pin attached to the "CLK" (or "A") pin of the encoder.
     * @param[in] dtPin GPIO pin attached to the "DT" (or "B") pin of the encoder.
     * @param[in] cwButtonNumber A number for the "virtual button" of a clockwise rotation event.
     * @param[in] ccwButtonNumber A number for the "virtual button" of a counter-clockwise rotation event.
     *                           If not given, `cwButtonNumber`+1 is used.
     * @param[in] useAlternateEncoding Set to true in order to use the signal encoding of
     *                                 ALPS RKJX series of rotary encoders, and the alike.
     * @note Internal pullup resistors will be enabled when available.
     */
    RotaryEncoderInput(
        InputGPIO clkPin,
        InputGPIO dtPin,
        InputNumber cwButtonNumber,
        InputNumber ccwButtonNumber,
        bool useAlternateEncoding = false);

    /**
     * @brief Set a time multiplier for "pulse" events
     *
     * @param multiplier A time multiplier between 1 and 6.
     *                   Invalid values are ignored.
     * @return true if the multiplier has changed.
     * @return false otherwise.
     */
    static bool setPulseMultiplier(uint8_t multiplier)
    {
        if ((multiplier > 0) && (multiplier < 7) && (pulseMultiplier != multiplier))
        {
            pulseMultiplier = multiplier;
            return true;
        }
        return false;
    };

    virtual uint64_t read(uint64_t lastState) override;
};

//-------------------------------------------------------------------
// Button matrix
//-------------------------------------------------------------------

/**
 * @brief Button matrix hardware
 *
 */
class ButtonMatrixInput : public DigitalInput
{
private:
    // InputGPIOCollection inputPins = {};
    // OutputGPIOCollection outputPins = {};
    ButtonMatrix matrix;
    bool negativeLogic;

public:
    /**
     * @brief Construct a new Button Matrix Input object
     *
     * @param matrix Button matrix specification
     * @param negativeLogic True to use negative logic
     */
    ButtonMatrixInput(
        const ButtonMatrix &matrix,
        bool negativeLogic = false);

    virtual uint64_t read(uint64_t lastState) override;
};

//-------------------------------------------------------------------
// Analog Multiplexers
//-------------------------------------------------------------------

/**
 * @brief State of switches connected to analog multiplexers
 *
 * @note Despite the use of analog multiplexers, input is digital (on/off)
 *
 */
class AnalogMultiplexerInput : public DigitalInput
{
private:
    OutputGPIOCollection selectorPins;
    InputGPIOCollection inputPins;
    const uint64_t *bitmap;
    size_t switchCount;

public:
    /**
     * @brief Construct a new Analog Multiplexer Input object
     *
     * @param selectorPin1 A selector pin
     * @param selectorPin2 A selector pin
     * @param selectorPin3 A selector pin
     * @param chips Group of chips
     */
    AnalogMultiplexerInput(
        OutputGPIO selectorPin1,
        OutputGPIO selectorPin2,
        OutputGPIO selectorPin3,
        const AnalogMultiplexerGroup<Mux8Pin> &chips);

    /**
     * @brief Construct a new Analog Multiplexer Input object
     *
     * @param selectorPin1 A selector pin
     * @param selectorPin2 A selector pin
     * @param selectorPin3 A selector pin
     * @param selectorPin4 A selector pin
     * @param chips Group of chips
     */
    AnalogMultiplexerInput(
        OutputGPIO selectorPin1,
        OutputGPIO selectorPin2,
        OutputGPIO selectorPin3,
        OutputGPIO selectorPin4,
        const AnalogMultiplexerGroup<Mux16Pin> &chips);

    /**
     * @brief Construct a new Analog Multiplexer Input object
     *
     * @param selectorPin1 A selector pin
     * @param selectorPin2 A selector pin
     * @param selectorPin3 A selector pin
     * @param selectorPin4 A selector pin
     * @param selectorPin5 A selector pin
     * @param chips Group of chips
     */
    AnalogMultiplexerInput(
        OutputGPIO selectorPin1,
        OutputGPIO selectorPin2,
        OutputGPIO selectorPin3,
        OutputGPIO selectorPin4,
        OutputGPIO selectorPin5,
        const AnalogMultiplexerGroup<Mux32Pin> &chips);

    virtual uint64_t read(uint64_t lastState) override;

private:
    void initializeMux();
};


//-------------------------------------------------------------------
// Coded rotary switch
//-------------------------------------------------------------------

/**
 * @brief Class for coded rotary switches
 *
 */
class RotaryCodedSwitchInput : public DigitalInput
{
private:
    InputGPIOCollection inputPins;
    uint64_t *bitmap = nullptr;
    bool complementaryCode;

public:

    /**
     * @brief Construct a new Rotary Coded Switch Input object
     *
     * @param spec Specification of input numbers assigned to switch positions
     * @param pins Collection of input pins for the binary encoding
     * @param complementaryCode If true, the input pins work in complementary binary encoding
     */
    RotaryCodedSwitchInput(
        const RotaryCodedSwitch &spec,
        const InputGPIOCollection &pins,
        bool complementaryCode);
    ~RotaryCodedSwitchInput();

    virtual uint64_t read(uint64_t lastState) override;
};

//-------------------------------------------------------------------
// I2C input hardware
//-------------------------------------------------------------------

/**
 * @brief Base class for switches/buttons attached to an I2C GPIO expander
 *
 * @note All buttons must work in negative logic (pulled up)
 */
class I2CInput : public DigitalInput // : public I2CInput
{
protected:
    /// @brief I2C address in 8 bits format (no need to shift left),
    ///        ready to overlap the R/W bit. By default, the R/W bit is set to "write".
    uint8_t deviceAddress;

    /// @brief Configured I2C bus
    I2CBus bus;

public:
    /**
     * @brief Construct a new I2CButtonsInput object
     *
     * @param address7Bits I2C address in 7 bits format
     * @param bus Bus where the chip is attached to
     * @param max_speed_mult Bus speed multiplier in the range from 1 to 4
     */
    I2CInput(
        uint8_t address7Bits,
        I2CBus bus = I2CBus::PRIMARY,
        uint8_t max_speed_mult = 1);
};

/**
 * @brief Class for buttons attached to a PCF8574 GPIO expander
 *
 */
class PCF8574ButtonsInput : public I2CInput
{
private:
    PCF8574Expander inputNumbers;
    bool getGPIOstate(uint64_t &state);

public:
    /**
     * @brief Construct a new PCF8574ButtonsInput object
     *
     * @param inputNumbers Specification of inputs
     * @param address7Bits I2C address in 7 bits format
     * @param bus Bus where the chip is attached to
     */
    PCF8574ButtonsInput(
        const PCF8574Expander &inputNumbers,
        uint8_t address7Bits,
        I2CBus bus = I2CBus::PRIMARY);

    virtual uint64_t read(uint64_t lastState) override;
};

/**
 * @brief Class for buttons attached to a MCP23017 GPIO expander
 *
 */
class MCP23017ButtonsInput : public I2CInput
{
private:
    MCP23017Expander inputNumbers;
    bool getGPIOstate(uint64_t &state);
    void configure();

public:
    /**
     * @brief Construct a new MCP23017ButtonsInput object
     *
     * @param inputNumbers Specification of inputs
     * @param address7Bits I2C address in 7 bits format
     * @param bus Bus where the chip is attached to
     */
    MCP23017ButtonsInput(
        const MCP23017Expander &inputNumbers,
        uint8_t address7Bits,
        I2CBus bus = I2CBus::PRIMARY);

    virtual uint64_t read(uint64_t lastState) override;
};

//-------------------------------------------------------------------
// Shift registers
//-------------------------------------------------------------------

/**
 * @brief State of switches connected to PISO shift registers
 *
 */
class ShiftRegistersInput : public DigitalInput
{
private:
    size_t switchCount;
    InputGPIO serialPin;
    OutputGPIO loadPin;
    OutputGPIO nextPin;
    const uint64_t *bitmap;
    bool loadHighOrLow;
    bool nextHighToLowOrLowToHigh;

public:
    /**
     * @brief Construct a new Shift Registers Input object
     *
     * @param loadPin GPIO number of the load pin
     * @param nextPin GPIO number of the next/clock pin
     * @param inputPin GPIO number of the serial output pin
     * @param chain Chain of PISO shift registers
     * @param SER_inputNumber Input number assigned to the SER pin
     *                        in the last chip of the chain.
     * @param loadHighOrLow If true, parallel inputs are loaded when `loadPin`is HIGH.
     *                      If false, parallel inputs are loaded when `loadPin`is LOW.
     * @param nextHighToLowOrLowToHigh If true, next bit is selected when an high-to-low
     *                                 pulse is detected at `nextPin`. If false, next bit
     *                                 is selected when a low-to-high pulse is detected.
     */
    ShiftRegistersInput(
        OutputGPIO loadPin,
        OutputGPIO nextPin,
        InputGPIO inputPin,
        const ShiftRegisterChain &chain,
        InputNumber SER_inputNumber = UNSPECIFIED::VALUE,
        const bool loadHighOrLow = false,
        const bool nextHighToLowOrLowToHigh = false);

    virtual uint64_t read(uint64_t lastState) override;
};

//-------------------------------------------------------------------
// Potentiometers
//-------------------------------------------------------------------

/**
 * @brief Class for all polled analog inputs (axis)
 *
 */
class AnalogInput
{
public:
    /**
     * @brief Get auto-calibration data. Required for persistent storage.
     *
     * @param[out] minReading Minimum adc reading
     * @param[out] maxReading Maximum adc reading
     */
    virtual void getCalibrationData(int &minReading, int &maxReading) = 0;

    /**
     * @brief Set auto-calibration data (loaded from persistent storage).
     *
     * @param[out] minReading Minimum adc reading
     * @param[out] maxReading Maximum adc reading
     */
    virtual void setCalibrationData(int minReading, int maxReading) = 0;

    /**
     * @brief Force auto-calibration.
     *
     */
    virtual void resetCalibrationData() = 0;

    /**
     * @brief Read current axis position.
     *        The axis must go from one end to the other
     *        for auto- calibration.
     *
     * @param[out] value Current axis position.
     * @param[out] autoCalibrated True if this axis has been auto-calibrated.
     */
    virtual void read(uint8_t &value, bool &autoCalibrated) = 0;

    virtual ~AnalogInput() noexcept {}
};

/**
 * @brief Class for analog clutch paddles
 *
 */
class AnalogClutchInput : public AnalogInput
{
protected:
    /// @brief Configured ADC pin
    ADC_GPIO pinNumber;
    /// @brief Minimum ADC reading for auto-calibration
    int minADCReading;
    /// @brief Maximum ADC reading for auto-calibration
    int maxADCReading;
    /// @brief Last ADC reading
    int lastADCReading;
    /// @brief Last axis position
    uint8_t lastValue;

public:
    /**
     * @brief Construct a new Analog Clutch Input object
     *
     * @param pinNumber ADC-capable pin number
     */
    AnalogClutchInput(ADC_GPIO pinNumber);

    void resetCalibrationData() override;

    void getCalibrationData(int &minReading, int &maxReading) override;

    void setCalibrationData(int minReading, int maxReading) override;

    void read(uint8_t &value, bool &autoCalibrated) override;
};

//-------------------------------------------------------------------
// Fake input hardware for testing
//-------------------------------------------------------------------

/**
 * @brief Fake digital input for testing
 *
 */
class FakeDigitalInput : public DigitalInput
{
private:
    FakeInput *_instance;

public:
    /**
     * @brief Construct a new Fake Digital Input object
     *
     * @param instance Fake input specification
     */
    FakeDigitalInput(FakeInput *instance) { _instance = instance; }

    virtual uint64_t read(uint64_t lastState) override
    {
        return _instance->state;
    }
};

/**
 * @brief Fake analog input for testing
 *
 */
class FakeAxis : public AnalogInput
{
private:
    FakeInput *_instance;
    bool _leftOrRight;

public:
    /**
     * @brief Construct a new Fake Digital Input object
     *
     * @param instance Fake input specification
     * @param leftOrRight True for the left axis,
     *                    false for the right axis
     */
    FakeAxis(FakeInput *instance, bool leftOrRight)
    {
        _leftOrRight = leftOrRight;
        _instance = instance;
    }

    void resetCalibrationData() override
    {
        _instance->recalibrationRequestCount++;
    };

    void getCalibrationData(int &minReading, int &maxReading) override
    {
        minReading = 0;
        maxReading = 254;
    }

    void setCalibrationData(int minReading, int maxReading) override {};

    void read(uint8_t &value, bool &autoCalibrated)
    {
        autoCalibrated = false;
        if (_leftOrRight)
            value = _instance->leftAxis;
        else
            value = _instance->rightAxis;
    };
};