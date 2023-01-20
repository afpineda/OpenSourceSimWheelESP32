/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-01-19
 * @brief Use of analog multiplexors as inputs
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#ifndef __ANALOGMULTIPLEXERINPUT_H__
#define __ANALOGMULTIPLEXERINPUT_H__

#include "PolledInput.h"
#include "SimWheelTypes.h"

/**
 * @brief State of switches connected to analog multiplexeres
 *
 * @note Despite the use of analog multiplexers, input is digital (on/off)
 *
 */
class AnalogMultiplexerInput : public DigitalPolledInput
{
private:
    uint8_t selectorPinCount, inputPinCount;
    const gpio_num_t *selectorPins;
    const gpio_num_t *inputPins;
    BaseType_t debounce;
    inputNumber_t *buttonNumbersArray;
    bool negativeLogic;
    uint8_t switchCount;

public:
    /**
     * @brief Construct a new Analog Multiplexer Input object
     *
     * @param selectorPins Array of GPIO numbers for selector pins.
     * @param selectorPinCount Length of `selectorPins` array.
     * @param inputPins Array of GPIO numbers for input pins.
     * @param inputPinCount Length of `inputPins`array.
     * @param buttonNumbersArray Array of input numbers to be assigned to every button.
     *                           The length of this array is expected to match 
     *                           (2^selectorPinCount)*inputPinCount.
     * @param negativeLogic If true (default), a switch is closed when LOW voltage is detected.
     *                      If false, a switch is closed when HIGH voltage is detected.
     * @param nextInChain Another instance to build a chain, or nullptr.
     */
    AnalogMultiplexerInput(
        const gpio_num_t selectorPins[],
        const uint8_t selectorPinCount,
        const gpio_num_t inputPins[],
        const uint8_t inputPinCount,
        inputNumber_t *buttonNumbersArray = nullptr,
        const bool negativeLogic = true,
        DigitalPolledInput *nextInChain = nullptr);

    /**
     * @brief Read the current state of multiplexed switches
     *
     * @param lastState Returned state of the previous call. This is required for debouncing.
     *        Set to zero at first call.
     * @return inputBitmap_t Current state of all switches, one bit per button.
     */
    virtual inputBitmap_t read(inputBitmap_t lastState) override;
};

#endif