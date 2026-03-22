/**
 * @file BatteryMonitorHardware.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-08-23
 * @brief Battery monitoring hardware
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "BatteryMonitorHardware.hpp"
#include "InternalServices.hpp"
#include "SimWheel.hpp"
#include "HAL.hpp"

//-------------------------------------------------------------------
// Hardware witnesses
//-------------------------------------------------------------------

void BatteryCharger::setExternalPowerWitness(
    InputGPIO sensePin,
    bool negativeLogic,
    bool enableInternalPullResistor)
{
    sensePin.reserve();
    BatteryCharger::_powerSensePin = sensePin;
    if (enableInternalPullResistor)
        internals::hal::gpio::forInput(sensePin, !negativeLogic, negativeLogic);
    else
        internals::hal::gpio::forInput(sensePin, false, false);
    BatteryCharger::_powerSenseNegativeLogic = negativeLogic;
}

void BatteryCharger::setChargingWitness(
    InputGPIO sensePin,
    bool negativeLogic,
    bool enableInternalPullResistor)
{
    sensePin.reserve();
    BatteryCharger::_chargingSensePin = sensePin;
    if (enableInternalPullResistor)
        internals::hal::gpio::forInput(sensePin, !negativeLogic, negativeLogic);
    else
        internals::hal::gpio::forInput(sensePin, false, false);
    BatteryCharger::_chargingSenseNegativeLogic = negativeLogic;
}

void BatteryCharger::update(BatteryStatus &status)
{
#if !CD_CI
    bool level;
    if (BatteryCharger::_powerSensePin != UNSPECIFIED::VALUE)
    {
        level = GPIO_GET_LEVEL(BatteryCharger::_powerSensePin);
        status.usingExternalPower =
            level ^ BatteryCharger::_powerSenseNegativeLogic;
    }
    if (BatteryCharger::_chargingSensePin != UNSPECIFIED::VALUE)
    {
        level = GPIO_GET_LEVEL(BatteryCharger::_chargingSensePin);
        status.isCharging =
            level ^ BatteryCharger::_chargingSenseNegativeLogic;
    }
#endif
}

//-------------------------------------------------------------------
// Abstract hardware interface
//-------------------------------------------------------------------

#define MAX_ATTEMPTS 20
#define CONSTANT_CURRENT_SOC_DELTA 25

void BatteryMonitorInterface::getStatus(BatteryStatus &currentStatus)
{
    // Take advantage of the charging witness if available
    currentStatus.reset();
    BatteryCharger::update(currentStatus);
    if (currentStatus.isCharging.has_value())
    {
        if (currentStatus.isCharging.value())
        {
            // The battery is charging for sure.
            // SoC is unknown. No need to guess.
            currentStatus.isCharging = true;
            currentStatus.stateOfCharge.reset();
            currentStatus.isBatteryPresent.reset();
            if (!currentStatus.usingExternalPower.has_value())
                currentStatus.usingExternalPower = true;
        }
        else
        {
            // The battery is not charging for sure.
            // No need to guess.
            uint8_t soc;
            if (read_soc(soc))
            {
                currentStatus.isCharging = false;
                currentStatus.isBatteryPresent = true;
                currentStatus.stateOfCharge = soc;
            }
            else
            {
                // No battery
                currentStatus.isBatteryPresent = false;
                currentStatus.isCharging = false;
                if (!currentStatus.usingExternalPower.has_value())
                    currentStatus.usingExternalPower = true;
            }
        }
        return;
    }

    // Guess whether the battery is charging or not
    bool seemsToBeCharging = false;
    uint8_t worstBatteryLevel = 255;
    uint8_t bestBatteryLevel = 0;
    uint8_t failureCount = 0;
    for (
        uint8_t attempt = 0;
        (attempt < MAX_ATTEMPTS) && !seemsToBeCharging;
        attempt++)
    {
        uint8_t soc = 0;
        if (read_soc(soc))
        {
            seemsToBeCharging = (soc > 101);
            if (soc < worstBatteryLevel)
                worstBatteryLevel = soc;
            if (soc > bestBatteryLevel)
                bestBatteryLevel = soc;
        }
        else
            failureCount++;
        DELAY_MS(50);
    }

    if (failureCount == MAX_ATTEMPTS)
    {
        // There is no battery
        currentStatus.isBatteryPresent = false;
        currentStatus.isCharging = false;
        if (!currentStatus.usingExternalPower.has_value())
            currentStatus.usingExternalPower = true;
        return;
    }

    // Guess constant current charging
    seemsToBeCharging = seemsToBeCharging || (failureCount > 0) ||
                        ((bestBatteryLevel > worstBatteryLevel) &&
                         (bestBatteryLevel - worstBatteryLevel) >=
                             CONSTANT_CURRENT_SOC_DELTA);

    if (seemsToBeCharging)
    {
        // There is no reliable SoC
        currentStatus.isCharging = true;
        currentStatus.stateOfCharge.reset();
        currentStatus.isBatteryPresent.reset();
        if (!currentStatus.usingExternalPower.has_value())
            currentStatus.usingExternalPower = true;
        return;
    }

    // There is a battery and is not charging
    currentStatus.isCharging = false;
    currentStatus.isBatteryPresent = true;
    currentStatus.stateOfCharge = worstBatteryLevel;
}

//-------------------------------------------------------------------
// MAX1704x hardware
//-------------------------------------------------------------------

#define I2C_TIMEOUT_MS 150

#define MAX1704x_I2C_ADDRESS 0x36
#define MAX1704x_REG_SoC 0x04
#define MAX1704x_REG_MODE 0x06
#define MAX1704x_REG_VERSION 0x08
#define MAX1704x_REG_CONFIG 0x0C

bool MAX1704x::read(uint8_t regAddress, uint16_t &value)
{
#if !CD_CI
    esp_err_t err = i2c_master_transmit_receive(
        I2C_SLAVE(device),
        &regAddress,
        1,
        (uint8_t *)&value,
        2,
        I2C_TIMEOUT_MS);
    if (err == ESP_OK)
    {
        // swap bytes because the MSB was transmitted first
        uint8_t aux = value >> 8;
        value = (value << 8) | aux;
    }
    return (err == ESP_OK);
#else
    return false;
#endif
}

bool MAX1704x::write(uint8_t regAddress, uint16_t value)
{
#if !CD_CI
    uint8_t buffer[3];
    buffer[0] = regAddress;
    buffer[1] = value >> 8; // MSB first
    buffer[2] = value;
    esp_err_t err = i2c_master_transmit(
        I2C_SLAVE(device),
        buffer,
        3,
        I2C_TIMEOUT_MS);
    return (err == ESP_OK);
#else
    return false;
#endif
}

bool MAX1704x::quickStart()
{
    return write(MAX1704x_REG_MODE, 0x4000);
}

bool MAX1704x::read_soc(uint8_t &currentSoC)
{
    uint16_t value;
    if (read(MAX1704x_REG_SoC, value))
    {
        uint8_t *data = (uint8_t *)&value;
        currentSoC = data[1];
        if (data[0] >= 127)
            currentSoC++;
        return true;
    }
    return false;
}

MAX1704x::MAX1704x(I2CBus bus, uint8_t i2c_address)
{
    if (i2c_address < 128)
        internals::hal::i2c::abortOnInvalidAddress(i2c_address);
    else
        i2c_address = MAX1704x_I2C_ADDRESS;
#if !CD_CI
    device = static_cast<void *>(
        internals::hal::i2c::add_device(i2c_address, 4, bus));
#endif
}

MAX1704x::~MAX1704x()
{
#if !CD_CI
    internals::hal::i2c::remove_device(I2C_SLAVE(device));
#endif
}

//-------------------------------------------------------------------
// Voltage divider hardware
//-------------------------------------------------------------------

#define VOLTAGE_SAMPLES_COUNT 100
#define NO_BATTERY_ADC_READING 150

int VoltageDividerMonitor::read()
{
#if !CD_CI
    // Enable circuit when required
    if (_batteryENPin != UNSPECIFIED::VALUE)
    {
        GPIO_SET_LEVEL(_batteryENPin, 1);
        DELAY_TICKS(100);
    }
#endif

    // Get ADC reading
    lastBatteryReading = internals::hal::gpio::getADCreading(
        _batteryREADPin,
        VOLTAGE_SAMPLES_COUNT);

#if !CD_CI
    // Disable circuit when required
    if (_batteryENPin != UNSPECIFIED::VALUE)
        GPIO_SET_LEVEL(_batteryENPin, 0);
#endif

    return lastBatteryReading;
}

uint8_t VoltageDividerMonitor::readingToSoC(int reading)
{
    int batteryLevel =
        BatteryCalibrationService::call().getBatteryLevel(reading);
    if (batteryLevel < 0)
    {
        // Battery calibration is *not* available
        // fallback to auto-calibration algorithm
        batteryLevel =
            BatteryCalibrationService::call().getBatteryLevelAutoCalibrated(
                reading);
    }
    return batteryLevel;
}

VoltageDividerMonitor::VoltageDividerMonitor(
    ADC_GPIO battREADPin,
    OutputGPIO battENPin,
    uint32_t resistorToGND,
    uint32_t resistorToBattery)
{
    if ((resistorToGND >= resistorToBattery) &&
        (resistorToGND > 0) && (resistorToBattery > 0))
    {
        // Note: 4300 is the minimum expected charging voltage in millivolts
        CHARGING_ADC_READING =
            (4300 * resistorToGND) /
            (resistorToGND + resistorToBattery); // in millivolts
        CHARGING_ADC_READING =
            CHARGING_ADC_READING * 4095 / 3300; // in ADC steps
    }
    else
        // Incoherent values: using the designed 200K and 110K resistors
        CHARGING_ADC_READING = 3442;

    battREADPin.reserve();
    internals::hal::gpio::forInput(battREADPin, false, false);
    _batteryREADPin = battREADPin;
    _batteryENPin = battENPin;
    if (battENPin != UNSPECIFIED::VALUE)
    {
        battENPin.reserve();
        internals::hal::gpio::forOutput(battENPin, false, false);
    }
}

bool VoltageDividerMonitor::read_soc(uint8_t &soc)
{
    int reading = read();
    if (reading <= NO_BATTERY_ADC_READING)
        return false;
    if (reading >= CHARGING_ADC_READING)
    {
        soc = 255;
        return true;
    }
    soc = readingToSoC(reading);
    return true;
}

//-------------------------------------------------------------------
