/**
 * @file ShiftRegistersInput.h
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-02-19
 * @brief Use of PISO shift registers as inputs
 *
 * @copyright Licensed under the EUPL
 *
 */

#ifndef __SHIFTREGISTERSINPUT_H__
#define __SHIFTREGISTERSINPUT_H__

#include "PolledInput.h"
#include "SimWheelTypes.h"

/**
 * @brief State of switches connected to PISO shift registers
 *
 */
class ShiftRegistersInput : public DigitalPolledInput
{
private:
    uint8_t switchCount;
    gpio_num_t loadPin, nextPin, serialPin;
    BaseType_t *debounce = nullptr;
    inputNumber_t *buttonNumbersArray;
    bool negativeLogic;
    bool loadHighOrLow;
    bool nextHighToLowOrLowToHigh;

public:
    /**
     * @brief Construct a new Shift Registers Input object
     *
     * @param serialPin GPIO number of the serial output pin
     * @param loadPin GPIO number of the load pin
     * @param nextPin GPIO number of the next/clock pin
     * @param buttonNumbersArray Array of switch numbers in the range 0-63
     * @param switchCount Count of switches or size of the previous array
     * @param negativeLogic If true, switches are ON when LOW voltage is detected.
     *                      If false, switches are OFF when LOW voltage is detected.
     * @param loadHighOrLow If true, parallel inputs are loaded when `loadPin`is HIGH.
     *                      If false, parallel inputs are loaded when `loadPin`is LOW.
     * @param nextHighToLowOrLowToHigh If true, next bit is selected when an high-to-low
     *                                 pulse is detected at `nextPin`. If false, next bit
     *                                 is selected when a low-to-high pulse is detected.
     * @param nextInChain Another instance to build a chain, or nullptr.
     */
    ShiftRegistersInput(
        const gpio_num_t serialPin,
        const gpio_num_t loadPin,
        const gpio_num_t nextPin,
        inputNumber_t *buttonNumbersArray,
        const uint8_t switchCount,
        const bool negativeLogic = true,
        const bool loadHighOrLow = false,
        const bool nextHighToLowOrLowToHigh = false,
        DigitalPolledInput *nextInChain = nullptr);

    /**
     * @brief Destroy the Shift Registers Input object
     *
     * @note Should not be called.
     */
    ~ShiftRegistersInput();

    /**
     * @brief Read the current state of all switches
     *
     * @param lastState Returned state of the previous call. This is required for debouncing.
     *        Set to zero at first call.
     * @return inputBitmap_t Current state of all switches, one bit per button.
     */
    virtual inputBitmap_t read(inputBitmap_t lastState) override;
};

#endif