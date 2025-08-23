/**
 * @file BatteryMonitorHardware.hpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-08-23
 * @brief Battery monitoring hardware
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "InternalTypes.hpp"
#include "SimWheelTypes.hpp"

//-------------------------------------------------------------------
// Abstract hardware interface
//-------------------------------------------------------------------

/**
 * @brief Interface to hardware implementing a battery monitor
 *
 */
class BatteryMonitorInterface
{
public:
    /**
     * @brief Get the battery status
     *
     * @note May be called before onStart()
     *
     * @param[out] currentStatus Current battery status
     */
    virtual void getStatus(BatteryStatus &currentStatus)
    {
        currentStatus.stateOfCharge.reset();
        currentStatus.isCharging.reset();
        currentStatus.isBatteryPresent.reset();
        currentStatus.usingExternalPower.reset();
    }

    /**
     * @brief Called once when the battery monitor daemon is started
     *
     */
    virtual void onStart() {};
};

//-------------------------------------------------------------------
// MAX1704x hardware
//-------------------------------------------------------------------

/**
 * @brief MAX1704x chips for battery monitoring
 *
 */
class MAX1704x : public BatteryMonitorInterface
{
protected:
    /// @brief Configured I2C address in 8 bit format
    uint8_t fg_i2c_address = 0xFF;

    /**
     * @brief Read from a register
     *
     * @param regAddress Register address
     * @param value Register value
     * @return true On success
     * @return false On failure
     */
    bool read(uint8_t regAddress, uint16_t &value);

    /**
     * @brief Write to a register
     *
     * @param regAddress Register address
     * @param value Value to write
     * @return true On success
     * @return false On failure
     */
    bool write(uint8_t regAddress, uint16_t value);

    /**
     * @brief Send a quick start command
     *
     * @return true On success
     * @return false On failure
     */
    bool quickStart();

    /**
     * @brief Read the state-of-charge register
     *
     * @param currentSoC Current state of charge
     * @return true On success
     * @return false On failure
     */
    bool read_SoC(uint8_t &currentSoC);

public:
    MAX1704x(I2CBus bus, uint8_t i2c_address);

    virtual void getStatus(BatteryStatus &currentStatus) override;

    virtual void onStart() override
    {
        quickStart();
    }

}; // class MAX1704x

//-------------------------------------------------------------------
// Voltage divider hardware
//-------------------------------------------------------------------

/**
 * @brief Battery monitor implemented as a voltage divider
 *
 */
class VoltageDividerMonitor : public BatteryMonitorInterface
{
protected:
    OutputGPIO _batteryENPin;
    ADC_GPIO _batteryREADPin;

public:
    // NOTE: these public members are for testing. Do not tamper with them.

    /// @brief Last ADC reading
    int lastBatteryReading = 0;

    /// @brief Get ADC reading
    int read();

public:
    VoltageDividerMonitor(ADC_GPIO battREADPin, OutputGPIO battENPin);

    /**
     * @brief Translate a battery reading to a state of charge
     *
     * @param reading ADC reading
     * @return uint8_t State of charge
     */
    static uint8_t readingToSoC(int reading);

    virtual void getStatus(BatteryStatus &currentStatus) override;
}; // class VoltageDividerMonitor

//-------------------------------------------------------------------
