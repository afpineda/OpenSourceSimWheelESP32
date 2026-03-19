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

#pragma once

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
    virtual void getStatus(BatteryStatus &currentStatus);

    /**
     * @brief Called once when the battery monitor daemon is started
     *
     */
    virtual void onStart() {};

protected:
    /**
     * @brief Retrieve the current state of charge
     *
     * @param[out] soc State of charge. Higher than 101 if the battery
     *                 is charging at constant voltage.
     * @return true If @p soc was retrieved with success
     * @return false If there is no battery or @p soc is unknown
     */
    virtual bool read_soc(uint8_t &soc) = 0;
};

//-------------------------------------------------------------------
// Optional hardware witnesses
//-------------------------------------------------------------------

/**
 * @brief Hardware witnesses regarding battery chargers
 *
 */
class BatteryCharger
{
public:
    /**
     * @brief Set a GPIO pin to sense the power wire
     *
     * @param sensePin GPIO pin to sense the power wire
     * @param negativeLogic If true, LOW voltage means there is external power
     *                      If false, HIGH voltage means there is
     *                      external power
     * @param enableInternalPullResistor If true, the corresponding internal
     *                                   pullup/pulldown resistor is enabled.
     *                                   Set to true if the wire sensing signal
     *                                   works in open drain.
     */
    static void setExternalPowerWitness(
        InputGPIO sensePin,
        bool negativeLogic,
        bool enableInternalPullResistor);

    /**
     * @brief Set a GPIO pin to sense if the battery is being charged
     *
     * @param sensePin GPIO pin attached to the battery charger chip
     * @param negativeLogic If true, LOW voltage means the battery is charging.
     *                      If false, HIGH voltage means the battery
     *                      is charging.
     * @param enableInternalPullResistor If true, the corresponding internal
     *                                   pullup/pulldown resistor is enabled.
     *                                   Set to true if the witness signal
     *                                   works in open drain.
     */
    static void setChargingWitness(
        InputGPIO sensePin,
        bool negativeLogic,
        bool enableInternalPullResistor);

    /**
     * @brief Update the battery status according to the witnesses
     *
     * @param[in,out] status Battery status
     */
    static void update(BatteryStatus &status);

private:
    inline static InputGPIO _powerSensePin{};
    inline static bool _powerSenseNegativeLogic = false;
    inline static InputGPIO _chargingSensePin{};
    inline static bool _chargingSenseNegativeLogic = false;
};

//-------------------------------------------------------------------
// Testing
//-------------------------------------------------------------------

/**
 * @brief Fake battery monitor for testing
 *
 */
class FakeBatteryMonitor : public BatteryMonitorInterface
{
public:
    /// @brief Pointer to variable that holds fake battery status
    BatteryStatus *status = nullptr;

public:
    /**
     * @brief Construct a new Fake Battery Monitor object
     *
     * @param fakeStatus Pointer to variable that holds fake battery status
     */
    FakeBatteryMonitor(BatteryStatus *fakeStatus)
    {
        this->status = fakeStatus;
    }

    virtual void getStatus(BatteryStatus &currentStatus) override
    {
        if (status != nullptr)
            currentStatus = *status;
    }

    virtual bool read_soc(uint8_t &soc) override { return false; }
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
    /// @brief I2C slave device (must be type-casted)
    void *device = nullptr;

    /**
     * @brief Read from a register
     *
     * @param regAddress Register address
     * @param[out] value Register value
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

    virtual bool read_soc(uint8_t &currentSoC) override;

public:
    /**
     * @brief Construct a new MAX1704x object
     *
     * @param bus I2C bus where the chip is attached
     * @param i2c_address 7-bit full I2C address.
     *                    Set to 0xFF to use a default address.
     */
    MAX1704x(I2CBus bus = I2CBus::PRIMARY, uint8_t i2c_address = 0xFF);

    /// @brief Destroy the MAX1704x object
    virtual ~MAX1704x();

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
    /// @brief ADC-capable GPIO for reading
    OutputGPIO _batteryENPin;
    /// @brief output GPIO to enable/disable the circuitry
    ADC_GPIO _batteryREADPin;
    /// @brief Minimum expected ADC reading when the battery is charging
    int CHARGING_ADC_READING = 3442;

    virtual bool read_soc(uint8_t &currentSoC) override;

public:
    // NOTE: these public members are for testing. Do not tamper with them.

    /// @brief Last ADC reading
    int lastBatteryReading = 0;

    /// @brief Get ADC reading
    int read();

public:
    /**
     * @brief Construct a new Voltage Divider Monitor object
     *
     * @note The parameters @p resistorToGND and @p resistorToBattery
     *       are used to determine the expected voltage in the ADC pin
     *       when charging, and nothing else.
     *       Incoherent values are ignored.
     *
     * @param battREADPin ADC-capable GPIO for reading
     * @param battENPin Output GPIO to enable or disable the circuit.
     *                  Set to -1 (GPIO_NUM_NC) if the NPN-PNP pair is not used.
     * @param resistorToGND Impedance of the resistor connected to GND.
     *                      Use any impedance unit, but the same as
     *                      @p resistorToBattery
     * @param resistorToBattery Impedance of the resistor connected to
     *                          the battery positive pole.
     *                          Use any impedance unit,
     *                          but the same as @p resistorToGND .
     *                          Must be lower than @p resistorToGND.
     */
    VoltageDividerMonitor(
        ADC_GPIO battREADPin,
        OutputGPIO battENPin,
        uint32_t resistorToGND = 200,
        uint32_t resistorToBattery = 110);

    /**
     * @brief Translate a battery reading to a state of charge
     *
     * @param reading ADC reading
     * @return uint8_t State of charge
     */
    static uint8_t readingToSoC(int reading);

}; // class VoltageDividerMonitor

//-------------------------------------------------------------------
