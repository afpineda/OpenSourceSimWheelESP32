/**
 * @file I2CExpanderInput.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-02-26
 * @brief Use of I2C GPIO expanders for input
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "I2CExpanderInput.h"

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

// MCP23017 registers

#define MCP23017_IO_CONFIGURATION 0x0A
#define MCP23017_IO_DIRECTION 0x00
#define MCP23017_PULL_UP_RESISTORS 0x0C
#define MCP23017_GPIO 0x12
#define MCP23017_POLARITY 0x02
#define MCP23017_INTERRUPT_ON_CHANGE 0x04
#define MCP23017_INTERRUPT_CONTROL 0x08
#define MCP23017_INTERRUPT_DEFAULT_VALUE 0x06

// ============================================================================
// Implementation of class: PCF8574ButtonsInput
// ============================================================================

PCF8574ButtonsInput::PCF8574ButtonsInput(
    uint8_t address7Bits,
    bool useSecondaryBus,
    DigitalPolledInput *nextInChain)
    : I2CButtonsInput(8, address7Bits, useSecondaryBus, nextInChain)
{
    // The PCF8574 does not have internal registers
    // Read GPIO registers in order to clear all interrupts
    inputBitmap_t dummy;
    getGPIOstate(dummy);
}

bool PCF8574ButtonsInput::getGPIOstate(inputBitmap_t &state)
{
    state = 0ULL;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, deviceAddress | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, (unsigned char *)&state, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    bool result = (i2c_master_cmd_begin(busDriver, cmd, DEBOUNCE_TICKS) == ESP_OK);
    i2c_cmd_link_delete(cmd);
    state = ~state; // convert to positive logic
    return result;
}

PCF8574InputSpec &PCF8574ButtonsInput::inputNumber(PCF8574_pin_t pin, inputNumber_t number)
{
    abortOnInvalidInputNumber(number);
    uint8_t index = static_cast<uint8_t>(pin);
    mask |= gpioBitmap[index];
    gpioBitmap[index] = BITMAP(number);
    mask &= ~gpioBitmap[index];
    return *this;
}

// ============================================================================
// Implementation of class: MCP23017ButtonsInput
// ============================================================================

MCP23017ButtonsInput::MCP23017ButtonsInput(
    uint8_t address7Bits,
    bool useSecondaryBus,
    DigitalPolledInput *nextInChain)
    : I2CButtonsInput(16, address7Bits, useSecondaryBus, nextInChain)
{
    i2c_cmd_handle_t cmd;

    // Configure IOCON register:
    // - Registers are in the same bank
    // - Interrupt pins mirrored
    // - Sequential operation
    // - Active driver output for interrupt pins
    // - Interrupt pins active low
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, deviceAddress, true);
    i2c_master_write_byte(cmd, MCP23017_IO_CONFIGURATION, true);
    i2c_master_write_byte(cmd, 0b01000000, true);
    i2c_master_stop(cmd);
    ESP_ERROR_CHECK(i2c_master_cmd_begin(busDriver, cmd, DEBOUNCE_TICKS * 10));
    i2c_cmd_link_delete(cmd);

    // Set mode to "input"
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, deviceAddress, true);
    i2c_master_write_byte(cmd, MCP23017_IO_DIRECTION, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_stop(cmd);
    ESP_ERROR_CHECK(i2c_master_cmd_begin(busDriver, cmd, DEBOUNCE_TICKS * 10));
    i2c_cmd_link_delete(cmd);

    // Enable pull-up resistors
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, deviceAddress, true);
    i2c_master_write_byte(cmd, MCP23017_PULL_UP_RESISTORS, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_stop(cmd);
    ESP_ERROR_CHECK(i2c_master_cmd_begin(busDriver, cmd, DEBOUNCE_TICKS * 10));
    i2c_cmd_link_delete(cmd);

    // Automatically convert negative logic to positive logic
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, deviceAddress, true);
    i2c_master_write_byte(cmd, MCP23017_POLARITY, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_stop(cmd);
    ESP_ERROR_CHECK(i2c_master_cmd_begin(busDriver, cmd, DEBOUNCE_TICKS * 10));
    i2c_cmd_link_delete(cmd);

    // Enable interrupts at all GPIO pins
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, deviceAddress, true);
    i2c_master_write_byte(cmd, MCP23017_INTERRUPT_ON_CHANGE, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_stop(cmd);
    ESP_ERROR_CHECK(i2c_master_cmd_begin(busDriver, cmd, DEBOUNCE_TICKS * 10));
    i2c_cmd_link_delete(cmd);

    // Trigger interrupts by comparison with DEFVAL registers
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, deviceAddress, true);
    i2c_master_write_byte(cmd, MCP23017_INTERRUPT_CONTROL, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_stop(cmd);
    ESP_ERROR_CHECK(i2c_master_cmd_begin(busDriver, cmd, DEBOUNCE_TICKS * 10));
    i2c_cmd_link_delete(cmd);

    // Set DEFVAL registers for interrupts
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, deviceAddress, true);
    i2c_master_write_byte(cmd, MCP23017_INTERRUPT_DEFAULT_VALUE, true);
    i2c_master_write_byte(cmd, 0, true); // Note: negative logic
    i2c_master_write_byte(cmd, 0, true);
    i2c_master_stop(cmd);
    ESP_ERROR_CHECK(i2c_master_cmd_begin(busDriver, cmd, DEBOUNCE_TICKS * 10));
    i2c_cmd_link_delete(cmd);

    // Read GPIO registers in order to clear all interrupts
    inputBitmap_t dummy;
    getGPIOstate(dummy);
}

bool MCP23017ButtonsInput::getGPIOstate(inputBitmap_t &state)
{
    state = 0ULL;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, deviceAddress, true);
    i2c_master_write_byte(cmd, MCP23017_GPIO, true);
    i2c_master_stop(cmd);
    bool result = (i2c_master_cmd_begin(busDriver, cmd, DEBOUNCE_TICKS) == ESP_OK);
    i2c_cmd_link_delete(cmd);
    if (result)
    {
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, deviceAddress | I2C_MASTER_READ, true);
        i2c_master_read(cmd, (uint8_t *)&state, 2, I2C_MASTER_LAST_NACK);
        i2c_master_stop(cmd);
        bool result = (i2c_master_cmd_begin(busDriver, cmd, DEBOUNCE_TICKS) == ESP_OK);
        i2c_cmd_link_delete(cmd);
    }
    return result;
}

MCP23017InputSpec &MCP23017ButtonsInput::inputNumber(MCP23017_pin_t pin, inputNumber_t number)
{
    abortOnInvalidInputNumber(number);
    uint8_t index = static_cast<uint8_t>(pin);
    mask |= gpioBitmap[index];
    gpioBitmap[index] = BITMAP(number);
    mask &= ~gpioBitmap[index];
    return *this;
}