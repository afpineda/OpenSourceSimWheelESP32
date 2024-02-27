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

class PCF8574ButtonsInput : public I2CButtonsInput
{
protected:
    virtual bool getGPIOstate(inputBitmap_t &state) override;

public:
    PCF8574ButtonsInput(
        uint8_t buttonsCount,
        inputNumber_t *buttonNumbersArray,
        uint8_t address7Bits,
        bool useSecondaryBus = false,
        DigitalPolledInput *nextInChain = nullptr);
};

class MCP23017ButtonsInput : public I2CButtonsInput
{
protected:
    virtual bool getGPIOstate(inputBitmap_t &state) override;

public:
    MCP23017ButtonsInput(
        uint8_t buttonsCount,
        inputNumber_t *buttonNumbersArray,
        uint8_t address7Bits,
        bool useSecondaryBus = false,
        DigitalPolledInput *nextInChain = nullptr);
};

#endif