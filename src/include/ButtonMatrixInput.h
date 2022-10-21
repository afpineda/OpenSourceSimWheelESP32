/**
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
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
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
class ButtonMatrixInput : public PolledInput
{
private:
    int selectorPinCount, inputPinCount;
    gpio_num_t selectorPin[MAX_MATRIX_SELECTOR_COUNT];
    gpio_num_t inputPin[MAX_MATRIX_INPUT_COUNT];
    BaseType_t debounce[MAX_MATRIX_SELECTOR_COUNT][MAX_MATRIX_INPUT_COUNT];
    inputNumber_t *buttonNumbersArray;
public:
    /**
     * @brief Construct a new Button Matrix Input object where input numbers are 
     *        correlative
     *
     * @param firstButtonNumber Assigned number to the first button. Following buttons
     *                          will get numbered in ascending order.
     * @param nextInChain Another instance to build a chain, or nullptr.
     */
    ButtonMatrixInput(int firstButtonNumber = 0, PolledInput *nextInChain = nullptr);

    /**
     * @brief Construct a new Button Matrix Input object where input numbers are
     *        explicitly given
     *
     * @param buttonNumbersArray A pointer to an array of input numbers. Array length should match
     *                           the product of the numbers of input pins and selector pins.
     *                           Mandatory (not null).
     * @param buttonsCount Number of items (buttons) in the previous array.
     * @param nextInChain Another instance to build a chain, or nullptr.
     */
    ButtonMatrixInput(inputNumber_t *buttonNumbersArray, uint8_t buttonsCount, PolledInput *nextInChain = nullptr );

    /**
     * @brief Set a selector (output) pin
     *
     * @param aPin GPIO number of a selector pin.
     *
     * @attention This method should not be called more times than MAX_MATRIX_SELECTOR_COUNT
     *            Do not call twice with the same parameter. Must be called before `read()`.
     */
    void addSelectorPin(gpio_num_t aPin);

    /**
     * @brief Set an input pin
     *
     * @param aPin GPIO number of an input
     *
     * @attention This method should not be called more times than MAX_MATRIX_INPUT_COUNT
     *            Do not call twice with the same parameter. Must be called before `read()`.
     */
    void addInputPin(gpio_num_t aPin);

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