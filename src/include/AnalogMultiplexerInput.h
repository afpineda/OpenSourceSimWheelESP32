/**
 * @file AnalogMultiplexerInput.h
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-01-19
 * @brief Use of analog multiplexors as inputs
 *
 * @copyright Licensed under the EUPL
 *
 */

#ifndef __ANALOGMULTIPLEXERINPUT_H__
#define __ANALOGMULTIPLEXERINPUT_H__

#include "PolledInput.h"
#include "SimWheelTypes.h"

/**
 * @brief State of switches connected to analog multiplexers
 *
 * @note Despite the use of analog multiplexers, input is digital (on/off)
 *
 */
class AnalogMultiplexerInput : public DigitalPolledInput,
                               public Multiplexers8InputSpec,
                               public Multiplexers16InputSpec,
                               public Multiplexers32InputSpec
{
private:
    uint8_t selectorPinCount, inputPinCount;
    gpio_num_t *selectorPins;
    gpio_num_t *inputPins;
    inputBitmap_t *bitmap;
    bool negativeLogic;
    uint8_t switchCount;

protected:
    void setBitmap(
        gpio_num_t inputPin,
        uint8_t chipPinIndex,
        inputNumber_t number);

public:
    /**
     * @brief Construct a new Analog Multiplexer Input object
     *
     * @param selectorPins Array of GPIO numbers for selector pins.
     * @param inputPins Array of GPIO numbers for input pins.
     * @param negativeLogic If true (default), a switch is closed when LOW voltage is detected.
     *                      If false, a switch is closed when HIGH voltage is detected.
     * @param nextInChain Another instance to build a chain, or nullptr.
     */
    AnalogMultiplexerInput(
        const gpio_num_array_t &selectorPins,
        const gpio_num_array_t &inputPins,
        const bool negativeLogic = true,
        DigitalPolledInput *nextInChain = nullptr);

    /**
     * @brief Destroy the Analog Multiplexer Input object
     *
     * @note Should not be called.
     */
    ~AnalogMultiplexerInput();

    /**
     * @brief Read the current state of multiplexed switches
     *
     * @param lastState Returned state of the previous call. This is required for debouncing.
     *        Set to zero at first call.
     * @return inputBitmap_t Current state of all switches, one bit per button.
     */
    virtual inputBitmap_t read(inputBitmap_t lastState) override;

public:
    virtual Multiplexers8InputSpec &inputNumber(
        gpio_num_t inputPin,
        mux8_pin_t pin,
        inputNumber_t number) override;

    virtual Multiplexers16InputSpec &inputNumber(
        gpio_num_t inputPin,
        mux16_pin_t pin,
        inputNumber_t number) override;

    virtual Multiplexers32InputSpec &inputNumber(
        gpio_num_t inputPin,
        mux32_pin_t pin,
        inputNumber_t number) override;
};

#endif