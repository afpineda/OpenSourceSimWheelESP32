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
#include <vector>
#include "driver/i2c.h"


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
     * @note If required, must be called before
     *       inputs::addPCF8574Digital(), inputs::addMCP23017Digital()
     *       or batteryMonitor::begin()
     *
     * @param sda SDA pin for the I2C bus.
     * @param scl SCL pin for the I2C bus.
     * @param secondaryBus True to initialize the secondary I2C bus,
     *                     false to initialize the primary I2C bus.
     */
    void begin(gpio_num_t sda, gpio_num_t scl, bool secondaryBus = false);

    /**
     * @brief Ensure the I2C bus is initialized
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
     * @note require() must be called first.
     *
     * @param address7bits I2C address of a slave device in 7 bits format.
     * @param secondaryBus True to use the secondary bus, false to use the primary bus.
     * @return true If the slave device is available and ready.
     * @return false If the slave device is not responding or
     *         the bus was not initialized.
     */
    bool probe(uint8_t address7bits, bool secondaryBus = false);

    /**
     * @brief Retrieve all devices available on an I2C bus.
     *
     * @note No need to call require().
     *       The bus will be initialized to minimum speed temporarily,
     *       then reinitialized to previous parameters (if any).
     *
     * @param[out] result List of addresses found, in 7-bit format.
     * @param[in] secondaryBus True to use the secondary bus, false to use the primary bus.
     */
    void probe(std::vector<uint8_t> &result, bool secondaryBus = false);

    /**
     * @brief Abort and reboot on an invalid I2C address
     *
     * @param address7bits I2C address to check in 7 bits format.
     * @param minAddress Start of custom valid address range, inclusive.
     * @param maxAddress End of custom valid address range, inclusive.
     */
    void abortOnInvalidAddress(uint8_t address7bits, uint8_t minAddress = 0, uint8_t maxAddress = 127);

    /**
     * @brief Find the full address of a device
     *
     * @param[in] fullAddressList A list of full addresses obtained from i2c::probe()
     * @param[in] hardwareAddress A partial 7-bit address
     * @param[in] hardwareAddressMask A mask. Bits set to 1 will be taken from @p hardwareAddress.
     *                            Bits set to 0 have to be found.
     *
     * @return 0xFF If no device was found matching the partial @p hardwareAddress
     * @return 0xFE If two or more devices where found matching the partial @p hardwareAddress
     * @return Otherwise, a full address in 7-bit format.
     */
    uint8_t findFullAddress(
        std::vector<uint8_t> &fullAddressList,
        uint8_t hardwareAddress,
        uint8_t hardwareAddressMask = 0b00000111);

    // ----------------------------------------------------------------------------

    inline i2c_port_t getBus(bool secondaryBus)
    {
#ifndef CONFIG_IDF_TARGET_ESP32C3
        return secondaryBus ? I2C_NUM_1 : I2C_NUM_0;
#else
        return I2C_NUM_0;
#endif
    }
}

#endif