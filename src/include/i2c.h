/**
 * @file i2c.h
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-08-16
 * @brief I2C bus initialization and common utilities
 *
 * @copyright Licensed under the EUPL
 *
 */

#ifndef __I2C_H__
#define __I2C_H__

#include "esp32-hal.h" // declares gpio_num_t

namespace i2c
{
    /**
     * @brief Initialize an I2C bus to certain pins.
     *
     * @note Must be called if your board does not feature a default I2C bus.
     *       Must be called if you want to initialize a secondary bus.
     *       Otherwise, there is no need to call, since the bus will be
     *       automatically initialized.
     *
     * @note If required, must be called before addPCF8574Digital(), addMCP23017Digital()
     *       or batteryMonitor::begin()
     *
     * @param sda SDA pin for the I2C bus.
     * @param scl SCL pin for the I2C bus.
     * @param secondaryBus True to initialize the secondary I2C bus,
     *                     false to initialize the primary I2C bus.
     */
    void begin(gpio_num_t sda, gpio_num_t scl, bool secondaryBus = false);

    /**
     * @brief Initialize an I2C bus when required
     *
     * @note Called from other namespaces. No need to call in user code.
     *
     * @param max_speed_multiplier Maximum clock speed multiplier supported in the range from 1 to 4.
     * @param secondaryBus True to initialize the secondary I2C bus,
     *                     false to initialize the primary I2C bus.
     */
    void require(uint8_t max_speed_multiplier = 4, bool secondaryBus = false);

    /**
     * @brief Check slave device availability on an I2C bus.
     *
     * @param address7bits I2C address of a slave device in 7 bits format.
     * @param bus True to use the secondary bus, false to use the primary bus.
     * @return true If the slave device is available and ready.
     * @return false If the slave device is not responding or
     *         the bus was not initialized.
     */
    bool probe(uint8_t address7bits, bool secondaryBus = false);
}

#endif