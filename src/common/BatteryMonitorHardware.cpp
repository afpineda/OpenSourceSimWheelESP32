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

#if !CD_CI
#include "driver/i2c.h" // For I2C operation
#endif

//-------------------------------------------------------------------
// MAX1704x hardware
//-------------------------------------------------------------------

#define I2C_WAIT_TICKS pdMS_TO_TICKS(150)

#define MAX1704x_I2C_ADDRESS_SHIFTED 0x6c
#define MAX1704x_REG_SoC 0x04
#define MAX1704x_REG_MODE 0x06
#define MAX1704x_REG_VERSION 0x08
#define MAX1704x_REG_CONFIG 0x0C

bool MAX1704x::read(uint8_t regAddress, uint16_t &value)
{
#if !CD_CI
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, fg_i2c_address | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, regAddress, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, fg_i2c_address | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, ((uint8_t *)&value) + 1, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, ((uint8_t *)&value), I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    bool result = (i2c_master_cmd_begin(I2C_NUM_0, cmd, I2C_WAIT_TICKS) == ESP_OK);
    i2c_cmd_link_delete(cmd);
    return result;
#else
    return false;
#endif
}

bool MAX1704x::write(uint8_t regAddress, uint16_t value)
{
#if !CD_CI
    uint8_t *data = (uint8_t *)&value;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, fg_i2c_address | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, regAddress, true);
    i2c_master_write_byte(cmd, data[1], true);
    i2c_master_write_byte(cmd, data[0], true);
    i2c_master_stop(cmd);
    bool result = (i2c_master_cmd_begin(I2C_NUM_0, cmd, I2C_WAIT_TICKS) == ESP_OK);
    i2c_cmd_link_delete(cmd);
    return result;
#else
    return false;
#endif
}

bool MAX1704x::quickStart()
{
    return write(MAX1704x_REG_MODE, 0x4000);
}

bool MAX1704x::read_SoC(uint8_t &currentSoC)
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
    {
        internals::hal::i2c::abortOnInvalidAddress(i2c_address);
        fg_i2c_address = (i2c_address << 1);
    }
    else
        fg_i2c_address = MAX1704x_I2C_ADDRESS_SHIFTED;
    internals::hal::i2c::require(4, bus);
}

void MAX1704x::getStatus(BatteryStatus &currentStatus)
{
    BatteryMonitorInterface::getStatus(currentStatus);
    uint8_t currentSoC;
    uint8_t worstBatteryLevel = 100;
    bool seemsToBeCharging = false;
    bool success = false;
    for (uint8_t i = 0; i < 10; i++)
    {
        if (read_SoC(currentSoC))
        {
            success = true;
            seemsToBeCharging = seemsToBeCharging || (currentSoC > 101);
            if (currentSoC < worstBatteryLevel)
                worstBatteryLevel = currentSoC;
        }
        DELAY_MS(100);
    }
    if (seemsToBeCharging)
    {
        // The battery is charging, so SoC is unknown
        currentStatus.isCharging = true;
        currentStatus.usingExternalPower = true;
    }
    else if (success)
    {
        // There is a battery and is not charging
        currentStatus.isCharging = false;
        currentStatus.isBatteryPresent = true;
        currentStatus.stateOfCharge = worstBatteryLevel;
    }
    else
    {
        // There is no battery
        currentStatus.isBatteryPresent = false;
        currentStatus.isCharging = false;
        currentStatus.usingExternalPower = true;
    }
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
    lastBatteryReading = internals::hal::gpio::getADCreading(_batteryREADPin, VOLTAGE_SAMPLES_COUNT);

#if !CD_CI
    // Disable circuit when required
    if (_batteryENPin != UNSPECIFIED::VALUE)
        GPIO_SET_LEVEL(_batteryENPin, 0);
#endif

    return lastBatteryReading;
}

uint8_t VoltageDividerMonitor::readingToSoC(int reading)
{
    int batteryLevel = BatteryCalibrationService::call::getBatteryLevel(reading);
    if (batteryLevel < 0)
    {
        // Battery calibration is *not* available
        // fallback to auto-calibration algorithm
        batteryLevel =
            BatteryCalibrationService::call::getBatteryLevelAutoCalibrated(reading);
    }
    return batteryLevel;
}

VoltageDividerMonitor::VoltageDividerMonitor(
    ADC_GPIO battREADPin,
    OutputGPIO battENPin,
    uint32_t resistorToGND,
    uint32_t resistorToBattery)
{
    if ((resistorToGND >= resistorToBattery) && (resistorToGND > 0) && (resistorToBattery > 0))
    {
        // Note: 4300 is the minimum expected charging voltage in millivolts
        CHARGING_ADC_READING = (4300 * resistorToGND) / (resistorToGND + resistorToBattery); // in millivolts
        CHARGING_ADC_READING = CHARGING_ADC_READING * 4095 / 3300;                           // in ADC steps
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

void VoltageDividerMonitor::getStatus(BatteryStatus &currentStatus)
{
    BatteryMonitorInterface::getStatus(currentStatus);
    uint8_t worstBatteryLevel = 100;
    bool seemsToBeCharging = false;
    bool success = false;
    for (uint8_t i = 0; i < 10; i++)
    {
        int reading = read();
        if (reading >= NO_BATTERY_ADC_READING)
        {
            success = true;
            seemsToBeCharging = seemsToBeCharging || (reading >= CHARGING_ADC_READING);
            // Note: we do not call readingToSoC() when the battery is charging
            // to keep the autocalibration algorithm from using the charging voltage as reference
            if (!seemsToBeCharging)
            {
                uint8_t currentSoC = readingToSoC(reading);
                if (currentSoC < worstBatteryLevel)
                    worstBatteryLevel = currentSoC;
            }
        }
        DELAY_MS(100);
    }
    if (seemsToBeCharging)
    {
        // The battery is charging, so SoC is unknown
        currentStatus.isBatteryPresent = true;
        currentStatus.isCharging = true;
        currentStatus.usingExternalPower = true;
    }
    else if (success)
    {
        // There is a battery and is not charging
        currentStatus.isCharging = false;
        currentStatus.isBatteryPresent = true;
        currentStatus.stateOfCharge = worstBatteryLevel;
    }
    else
    {
        // There is no battery
        currentStatus.isBatteryPresent = false;
        currentStatus.isCharging = false;
        currentStatus.usingExternalPower = true;
    }
}

//-------------------------------------------------------------------
