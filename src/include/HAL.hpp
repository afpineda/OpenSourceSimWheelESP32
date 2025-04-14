/**
 * @file HAL.hpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-11
 * @brief Hardware abstration and low-level utilities
 *
 * @copyright Licensed under the EUPL
 *
 */

#pragma once

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "InternalTypes.hpp"
#include "SimWheelTypes.hpp"

//-------------------------------------------------------------------
// GLOBALS
//-------------------------------------------------------------------

/// @brief Cast GPIO to an ESP32 pin number
#define AS_GPIO(pin) static_cast<gpio_num_t>((int)(pin))
/// @brief Cast I2CBus to an ESP32 port number
#define AS_PORT(bus) static_cast<i2c_port_t>(bus)
/// @brief Macro to write a logic level in a GPIO pin
#define GPIO_SET_LEVEL(pin, level) gpio_set_level(static_cast<gpio_num_t>((int)(pin)), (level))
/// @brief Macro to read the logic level in a GPIO pin
#define GPIO_GET_LEVEL(pin) gpio_get_level(static_cast<gpio_num_t>((int)(pin)))
/// @brief Interrupt Service Routine
typedef void (*ISRHandler)(void *arg);

#if !CD_CI
/// @brief Macro to wait in tick units
#define DELAY_TICKS(ticks) vTaskDelay(ticks)
/// @brief Macro to wait in millisecond units
#define DELAY_MS(ms) vTaskDelay(pdMS_TO_TICKS(ms))
#else
#define DELAY_TICKS(ticks) std::this_thread::sleep_for(std::chrono::microseconds(ticks))
#define DELAY_MS(ms) std::this_thread::sleep_for(std::chrono::milliseconds(ms))
#endif

//-------------------------------------------------------------------
// Exceptions
//-------------------------------------------------------------------

/**
 * @brief Exception for I2C bus initializaiton failure
 *
 */
class i2c_error : public std::runtime_error
{
public:
    /**
     * @brief Unable to initialize the I2C bus exception
     *
     * @param sda SDA pin number
     * @param scl SCL pin number
     * @param bus I2C bus
     * @param clock_mult Bus clock multiplier
     */
    i2c_error(int sda, int scl, int bus, int clock_mult)
        : std::runtime_error(
              "I2C: unable to initialize bus. SDA=" +
              std::to_string(sda) +
              " SCL=" +
              std::to_string(scl) +
              " BUS=" +
              std::to_string(bus) +
              " CLOCK=x" +
              std::to_string(clock_mult)) {}

    /**
     * @brief Invalid I2C address exception
     *
     * @param address I2C full address
     */

    i2c_error(uint8_t address)
        : std::runtime_error(
              "I2C: invalid address " +
              std::to_string((int)address) +
              " (dec)") {}

    virtual ~i2c_error() noexcept {}
};

/**
 * @brief Exception for I2C devices not found
 *
 */
class i2c_device_not_found : public std::runtime_error
{
public:
    /**
     * @brief Construct a new i2c_device_not_found exception
     *
     * @param address I2C hardware or full address
     * @param bus I2C bus
     */
    i2c_device_not_found(uint8_t address, int bus = 0)
        : std::runtime_error(
              "I2C: device not found, but required. Bus=" +
              std::to_string(bus) +
              " Hw/Full address=" +
              std::to_string(address) +
              " (dec)") {}

    virtual ~i2c_device_not_found() noexcept {}
};

/**
 * @brief Exception for unknown full I2C address
 *
 */
class i2c_full_address_unknown : public std::runtime_error
{
public:
    /**
     * @brief Construct a new i2c_full_address_unknown exception
     *
     * @param hwAddress I2C hardware address
     * @param bus I2C bus
     */
    i2c_full_address_unknown(uint8_t hwAddress, int bus = 0)
        : std::runtime_error(
              "I2C: unable to detect full address. Bus=" +
              std::to_string(bus) +
              " HW address=" +
              std::to_string(hwAddress) +
              " (dec)") {}

    virtual ~i2c_full_address_unknown() noexcept {}
};

//-------------------------------------------------------------------
// API
//-------------------------------------------------------------------

namespace internals
{
    //---------------------------------------------------------------
    // Hardware abstraction
    //---------------------------------------------------------------

    namespace hal
    {

        //---------------------------------------------------------------
        // I2C bus operation
        //---------------------------------------------------------------

        namespace i2c
        {
            /**
             * @brief Initialize an I2C bus to certain pins.
             *
             * @note Must be called if you want to initialize a secondary bus.
             *       Otherwise, there is no need to call, since the bus will be
             *       automatically initialized.
             *
             * @note If required, must be called before using any I2C hardware.
             *
             * @param sda SDA pin for the I2C bus.
             * @param scl SCL pin for the I2C bus.
             * @param bus I2C bus to initialize.
             */
            void initialize(
                GPIO sda,
                GPIO scl,
                I2CBus bus);

            /**
             * @brief Ensure the I2C bus is initialized
             *
             * @note Called from other namespaces. No need to call in user code.
             *
             * @param max_speed_multiplier Maximum clock speed multiplier
             *                             supported in the range from 1 to 4.
             * @param bus I2C bus required.
             */
            void require(
                uint8_t max_speed_multiplier = 4,
                I2CBus bus = I2CBus::PRIMARY);

            /**
             * @brief Check slave device availability on an I2C bus.
             *
             * @note require() must be called first.
             *
             * @param address7bits I2C address of a slave device in 7 bits format.
             * @param bus I2C bus.
             * @return true If the slave device is available and ready.
             * @return false If the slave device is not responding or
             *         the bus was not initialized.
             */
            bool probe(
                uint8_t address7bits,
                I2CBus bus = I2CBus::PRIMARY);

            /**
             * @brief Retrieve all devices available on an I2C bus.
             *
             * @note No need to call require().
             *       The bus will be initialized to minimum speed temporarily,
             *       then reinitialized to previous parameters (if any).
             *
             * @param[out] result List of addresses found,
             *                    in 7-bit format.
             * @param[in] bus I2C bus.
             */
            void probe(
                std::vector<uint8_t> &result,
                I2CBus bus = I2CBus::PRIMARY);

            /**
             * @brief Abort and reboot on an invalid I2C address
             *
             * @param address7bits I2C address to check in 7 bits format.
             * @param minAddress Start of custom valid address range,
             *                   inclusive.
             * @param maxAddress End of custom valid address range,
             *                  inclusive.
             */
            void abortOnInvalidAddress(
                uint8_t address7bits,
                uint8_t minAddress = 0,
                uint8_t maxAddress = 127);

            /**
             * @brief Find the full address of a device
             *
             * @param[in] fullAddressList A list of full addresses obtained from i2c::probe()
             * @param[in] hardwareAddress A partial 7-bit address
             * @param[in] hardwareAddressMask A mask.
             *                            Bits set to 1 will be taken from @p hardwareAddress.
             *                            Bits set to 0 have to be found.
             *
             * @return 0xFF If no device was found matching
             *              the partial @p hardwareAddress
             * @return 0xFE If two or more devices where found
             *              matching the partial @p hardwareAddress
             * @return uint8_t Otherwise, a full address in 7-bit format.
             */
            uint8_t findFullAddress(
                std::vector<uint8_t> &fullAddressList,
                uint8_t hardwareAddress,
                uint8_t hardwareAddressMask = 0b00000111);
        } // namespace i2c

        //---------------------------------------------------------------
        // GPIO operation
        //---------------------------------------------------------------

        namespace gpio
        {
            /**
             * @brief Get the mean of some continuous ADC readings.
             *
             * @param pin Pin number. Must be ADC-capable.
             * @param sampleCount Number of continuous ADC samples.
             *                    Pass 1 for a single reading (default).
             *                    Must be greater than zero.
             * @return int Mean of all continuous ADC samples or
             *             -1 if @p sampleCount is not greater than zero.
             */
            int getADCreading(ADC_GPIO pin, int sampleCount = 1);

            /**
             * @brief Configure a pin for output
             *
             * @param pin Pin number
             * @param initialLevel If true, set to HIGH after initialization.
             *                     Otherwise, set to LOW after initialization.
             * @param openDrain If true, configure in open drain mode.
             *                  If false, configure in output mode.
             */
            void forOutput(OutputGPIO pin, bool initialLevel, bool openDrain);

            /**
             * @brief Configure a pin for digital input
             *
             * @param pin
             * @param enablePullDown
             * @param enablePullUp
             */
            void forInput(
                InputGPIO pin,
                bool enablePullDown,
                bool enablePullUp);

#if CD_CI
            /**
             * @brief Inject a fake reading to getADCreading() for testing
             *
             */
            extern int fakeADCreading;
#endif

            /**
             * @brief Enable an interrupt service routine
             *
             * @param pin Pin
             * @param handler Routine
             * @param param Parameter to @p handler
             */
            void enableISR(InputGPIO pin, ISRHandler handler, void *param = nullptr);

            /**
             * @brief Wait for signal propagation
             *
             * @note This is active wait with no context switching
             *
             * @param nanoseconds Time to wait in nanoseconds
             */
            void wait_propagation(uint32_t nanoseconds);

        } // namespace gpio
    } // namespace hal
} // namespace internals
