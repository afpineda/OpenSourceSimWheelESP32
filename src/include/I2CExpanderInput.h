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
    virtual uint8_t getMaxGPIOCount() override { return 8; };
    virtual void initialize() override {};
};

class MCP23017ButtonsInput : public I2CButtonsInput
{
protected:
    virtual bool getGPIOstate(inputBitmap_t &state) override;
    virtual uint8_t getMaxGPIOCount() override { return 16; };
    virtual void initialize() override;
};

#endif