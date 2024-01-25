/**
 * @file ButtonMatrixInput.h
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-03
 * @brief Use of a switch/button matrix as inputs
 *
 * @section DESCRIPTION
 *
 * A button matrix is composed by selector (output) pins and input pins, sometimes they are
 * called row and column pins. In order to read the full state of the matrix,
 * this algorithm applies:
 * 1. Set one selector pin to HIGH
 * 2. Read all inputs
 * 3. Set the previous selector to LOW.
 * 4. Move to the next selector pin, and back to 1.
 *
 * @copyright Licensed under the EUPL
 *
 */

#ifndef __BUTTONMATRIXINPUT_H__
#define __BUTTONMATRIXINPUT_H__

#include "PolledInput.h"
#include "SimWheelTypes.h"

#define MAX_MATRIX_SELECTOR_COUNT 8
#define MAX_MATRIX_INPUT_COUNT 8

/**
 * @brief Setup and read the state of a button matrix
 *
 */
class ButtonMatrixInput : public DigitalPolledInput
{
private:
    uint8_t selectorPinCount, inputPinCount;
    const gpio_num_t *selectorPins;
    const gpio_num_t *inputPins;
    BaseType_t debounce[MAX_MATRIX_SELECTOR_COUNT][MAX_MATRIX_INPUT_COUNT];
    inputNumber_t *buttonNumbersArray;
    inputNumber_t alternateFirstInputNumber;

public:

    /**
     * @brief Construct a new Button Matrix Input object
     *
     * @param selectorPins Array of GPIO numbers for selector pins.
     * @param selectorPinCount Length of `selectorPins` array.
     * @param inputPins Array of GPIO numbers for input pins
     * @param inputPinCount Length of `inputPins`array.
     * @param buttonNumbersArray Array of input numbers to be assigned to every button.
     *                           The length of this array is expected to match the
     *                           product of `selectorPinCount` and `inputPinCount`. If NULL,
     *                           `alternateFirstInputNumber` is used instead.
     * @param alternateFirstInputNumber Assign a number to every button starting from this
     *                                  parameter, in ascending order. Ignored if `buttonNumbersArray`
     *                                  is not null.
     * @param nextInChain Another instance to build a chain, or nullptr.
     */
    ButtonMatrixInput(
        const gpio_num_t selectorPins[],
        const uint8_t selectorPinCount,
        const gpio_num_t inputPins[],
        const uint8_t inputPinCount,
        inputNumber_t *buttonNumbersArray = nullptr,
        inputNumber_t alternateFirstInputNumber = UNSPECIFIED_INPUT_NUMBER,
        DigitalPolledInput *nextInChain = nullptr);

    /**
     * @brief Read the current state of the matrix
     *
     * @param lastState Returned state of the previous call. This is required for debouncing.
     *        Set to zero at first call.
     * @return inputBitmap_t Current state of the matrix, one bit per button.
     */
    virtual inputBitmap_t read(inputBitmap_t lastState) override;
};

#endif