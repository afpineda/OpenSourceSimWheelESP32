/**
 * @file I2CExpanderInput.h
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-02-26
 * @brief Use of I2C GPIO expanders for input
 *
 * @copyright Licensed under the EUPL
 *
 */

#ifndef __I2C_EXPANDER_INPUT_H__
#define __I2C_EXPANDER_INPUT_H__

#include "PolledInput.h"

/**
 * @brief Class for buttons attached to a PCF8574 GPIO expander
 *
 */
class PCF8574ButtonsInput : public I2CButtonsInput
{
protected:
    virtual bool getGPIOstate(inputBitmap_t &state) override;

public:
    /**
     * @brief Construct a new PCF8574ButtonsInput object
     *
     * @param buttonNumbersArray Array of input numbers for the attached buttons. Length is 8.
     * @param address7Bits I2C address in 7 bits format.
     * @param useSecondaryBus TRUE to use the secondary bus, FALSE to use the primary bus.
     * @param nextInChain Another instance to build a chain, or nullptr.
     */
    PCF8574ButtonsInput(
        inputNumber_t *buttonNumbersArray,
        uint8_t address7Bits,
        bool useSecondaryBus = false,
        DigitalPolledInput *nextInChain = nullptr);
};

/**
 * @brief Class for buttons attached to a MCP23017 GPIO expander
 *
 */
class MCP23017ButtonsInput : public I2CButtonsInput
{
protected:
    virtual bool getGPIOstate(inputBitmap_t &state) override;

public:
    /**
     * @brief Construct a new MCP23017ButtonsInput object
     *
     * @param buttonNumbersArray Array of input numbers for the attached buttons. Length is 16.
     * @param address7Bits I2C address in 7 bits format.
     * @param useSecondaryBus TRUE to use the secondary bus, FALSE to use the primary bus.
     * @param nextInChain Another instance to build a chain, or nullptr.
     */
    MCP23017ButtonsInput(
        inputNumber_t *buttonNumbersArray,
        uint8_t address7Bits,
        bool useSecondaryBus = false,
        DigitalPolledInput *nextInChain = nullptr);
};

#endif