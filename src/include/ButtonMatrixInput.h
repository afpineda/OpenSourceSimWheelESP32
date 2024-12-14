/**
 * @file ButtonMatrixInput.h
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-03
 * @brief Use of a switch/button matrix as inputs
 *
 * @details A button matrix is composed by selector (output) pins and input pins,
 *          sometimes they are called row and column pins.
 *          In order to read the full state of the matrix,
 *          this algorithm applies:
 *          1. Set one selector pin to HIGH
 *          2. Read all inputs
 *          3. Set the previous selector to LOW.
 *          4. Move to the next selector pin, and back to 1.
 *
 * @copyright Licensed under the EUPL
 *
 */

#ifndef __BUTTON_MATRIX_INPUT_H__
#define __BUTTON_MATRIX_INPUT_H__

#include "PolledInput.h"
#include "SimWheelTypes.h"

/**
 * @def MAX_MATRIX_SELECTOR_COUNT
 * @brief Maximum selector pins allowed
 */
#define MAX_MATRIX_SELECTOR_COUNT 8

/**
 * @def MAX_MATRIX_INPUT_COUNT
 * @brief Maximum input pins allowed
 */
#define MAX_MATRIX_INPUT_COUNT 8

/**
 * @brief Setup and read the state of a button matrix
 *
 */
class ButtonMatrixInput : public DigitalPolledInput, public ButtonMatrixInputSpec
{
private:
    uint8_t selectorPinCount, inputPinCount;
    gpio_num_t selectorPins[MAX_MATRIX_SELECTOR_COUNT];
    gpio_num_t inputPins[MAX_MATRIX_INPUT_COUNT];
    inputBitmap_t *bitmap;

public:
    /**
     * @brief Construct a new Button Matrix Input object
     *
     * @param selectorPins Array of GPIO numbers for selector pins.
     * @param inputPins Array of GPIO numbers for input pins
     * @param nextInChain Another instance to build a chain, or nullptr.
     */
    ButtonMatrixInput(
        const gpio_num_array_t selectorPins,
        const gpio_num_array_t inputPins,
        DigitalPolledInput *nextInChain = nullptr);

    /**
     * @brief Destroy the Button Matrix Input object
     *
     * @note Should not be called.
     */
    ~ButtonMatrixInput();

    /**
     * @brief Read the current state of the matrix
     *
     * @param lastState Returned state of the previous call. This is required for debouncing.
     *        Set to zero at first call.
     * @return inputBitmap_t Current state of the matrix, one bit per button.
     */
    virtual inputBitmap_t read(inputBitmap_t lastState) override;

    virtual ButtonMatrixInputSpec &inputNumber(
        gpio_num_t selectorPin,
        gpio_num_t inputPin,
        inputNumber_t number) override;
};

#endif