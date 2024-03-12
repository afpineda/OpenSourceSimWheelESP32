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
class ShiftRegistersInput : public DigitalPolledInput, public ShiftRegisters8InputSpec
{
private:
    uint8_t switchCount;
    gpio_num_t loadPin, nextPin, serialPin;
    inputBitmap_t *bitmap;
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
     * @param switchCount Count of switches
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

    virtual ShiftRegisters8InputSpec &inputNumber(
        uint8_t indexInChain,
        sr8_pin_t pin,
        inputNumber_t number) override;
};

#endif