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

#define MCP23017_IO_DIRECTION 0x00
// #define MCP23017_CONFIG 0x05
#define MCP23017_PULL_UP_RESISTORS 0x0C
#define MCP23017_GPIO 0x12

// ============================================================================
// Implementation of class: PCF8574ButtonsInput
// ============================================================================

bool PCF8574ButtonsInput::getGPIOstate(inputBitmap_t &state)
{
    state = 0ULL;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (deviceAddress << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &state, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    bool result = (i2c_master_cmd_begin(busDriver, cmd, DEBOUNCE_TICKS) == ESP_OK);
    i2c_cmd_link_delete(cmd);
    return result;
}

// ============================================================================
// Implementation of class: MCP23017ButtonsInput
// ============================================================================

void MCP23017ButtonsInput::initialize()
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // Set mode to "input"
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (deviceAddress << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, MCP23017_IO_DIRECTION, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_stop(cmd);
    ESP_ERROR_CHECK(i2c_master_cmd_begin(busDriver, cmd, DEBOUNCE_TICKS * 10));
    i2c_cmd_link_delete(cmd);

    // Enable pull-up resistors
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (deviceAddress << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, MCP23017_PULL_UP_RESISTORS, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_write_byte(cmd, 0xFF, true);
    i2c_master_stop(cmd);
    ESP_ERROR_CHECK(i2c_master_cmd_begin(busDriver, cmd, DEBOUNCE_TICKS * 10));
    i2c_cmd_link_delete(cmd);
}

bool MCP23017ButtonsInput::getGPIOstate(inputBitmap_t &state)
{
    state = 0ULL;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (deviceAddress << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, MCP23017_GPIO, true);
    i2c_master_stop(cmd);
    bool result = (i2c_master_cmd_begin(busDriver, cmd, DEBOUNCE_TICKS) == ESP_OK);
    i2c_cmd_link_delete(cmd);
    if (result)
    {
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (deviceAddress << 1) | I2C_MASTER_READ, true);
        i2c_master_read(cmd, (uint8_t *)&state, 2, I2C_MASTER_LAST_NACK);
        i2c_master_stop(cmd);
        result = (i2c_master_cmd_begin(busDriver, cmd, DEBOUNCE_TICKS) == ESP_OK);
        i2c_cmd_link_delete(cmd);
    }
    return result;
}