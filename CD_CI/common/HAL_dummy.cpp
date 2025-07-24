/**
 * @file HAL_dummy.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-05
 * @brief Abstraction of the underlying ESP-IDF API
 *
 * @copyright Licensed under the EUPL
 *
 */

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "HAL.hpp"

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

int internals::hal::gpio::fakeADCreading;

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// I2C
//-------------------------------------------------------------------
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// I2C: Probe
//-------------------------------------------------------------------

bool internals::hal::i2c::probe(uint8_t address7bits, I2CBus bus)
{
    return true;
}

// ----------------------------------------------------------------------------

void internals::hal::i2c::probe(std::vector<uint8_t> &result, I2CBus bus)
{
    result.clear();
    for (uint8_t address = 0; address < 128; address++)
    {
        result.push_back(address);
    }
}

// ----------------------------------------------------------------------------
// I2C: _bus initialization
// ----------------------------------------------------------------------------

void internals::hal::i2c::initialize(GPIO sda, GPIO scl, I2CBus bus, bool enableInternalPullup)
{
}

void internals::hal::i2c::require(uint8_t max_speed_multiplier, I2CBus bus)
{
}

// ----------------------------------------------------------------------------
// I2C Checks
// ----------------------------------------------------------------------------

void internals::hal::i2c::abortOnInvalidAddress(
    uint8_t address7bits,
    uint8_t minAddress,
    uint8_t maxAddress)
{
}

// ----------------------------------------------------------------------------
// Hardware addresses
// ----------------------------------------------------------------------------

uint8_t internals::hal::i2c::findFullAddress(
    std::vector<uint8_t> &fullAddressList,
    uint8_t hardwareAddress,
    uint8_t hardwareAddressMask)
{
    return hardwareAddress;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// GPIO
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

int internals::hal::gpio::getADCreading(ADC_GPIO pin, int sampleCount)
{
    return internals::hal::gpio::fakeADCreading;
}

void internals::hal::gpio::forOutput(
    OutputGPIO pin,
    bool initialLevel,
    bool openDrain)
{
}

void internals::hal::gpio::forInput(
    InputGPIO pin,
    bool enablePullDown,
    bool enablePullUp)
{
}

void internals::hal::gpio::wait_propagation(uint32_t nanoseconds)
{
    std::this_thread::sleep_for(std::chrono::nanoseconds(nanoseconds));
}