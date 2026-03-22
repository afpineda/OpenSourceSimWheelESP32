/**
 * @file inputMap.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Translates firmware-defined input numbers
 *        to user-defined input numbers.
 *
 * @copyright Licensed under the EUPL
 *
 */

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include <array>
#include <vector>
// #include <iostream> // For testing

//-------------------------------------------------------------------
// GLOBALS
//-------------------------------------------------------------------

struct DefaultMap
{
    uint8_t firmware;
    uint8_t noAlt;
    uint8_t alt;
};

static std::array<uint8_t, 128> mapNoAlt;
static std::array<uint8_t, 128> mapAlt;
static std::vector<DefaultMap> defaultMap;
static bool computeOptimal = false;
static uint8_t max_firmware_in = 128;

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Internal API
//-------------------------------------------------------------------
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// Service class
//-------------------------------------------------------------------

class InputMapServiceProvider : public InputMapService
{
    virtual void setMap(
        uint8_t firmware_defined,
        uint8_t user_defined,
        uint8_t user_defined_alt) override
    {
        if ((firmware_defined < 128) &&
            (user_defined < 128) &&
            (user_defined_alt < 128))
        {
            mapNoAlt[firmware_defined] = user_defined;
            mapAlt[firmware_defined] = user_defined_alt;
        }
    }

    virtual void getMap(
        uint8_t firmware_defined,
        uint8_t &user_defined,
        uint8_t &user_defined_alt) override
    {
        if (firmware_defined < 128)
        {
            user_defined = mapNoAlt[firmware_defined];
            user_defined_alt = mapAlt[firmware_defined];
        }
        else
        {
            user_defined = 0xFF;
            user_defined_alt = 0xFF;
        }
    }

    virtual void resetMap() override
    {
        // Create an absolute default map
        for (uint8_t i = 0; i < 128; i++)
        {
            mapNoAlt[i] = i;
            mapAlt[i] = (i + 64) % 128;
        }
        // Override with custom defaults
        for (auto defMap : defaultMap)
        {
            mapNoAlt[defMap.firmware] = defMap.noAlt;
            mapAlt[defMap.firmware] = defMap.alt;
        }
    }
};

//-------------------------------------------------------------------
// Get started
//-------------------------------------------------------------------

void inputMapStart()
{
    // Due to rotary coded switches
    // the check for invalid firmware-defined input numbers
    // is delayed to the OnStart event
    for (auto defMap : defaultMap)
        if (!InputNumber::booked(defMap.firmware))
            throw std::runtime_error(
                "The input number " +
                std::to_string(defMap.firmware) +
                " can not be mapped, since it is not assigned");

    // Compute the highest firmware-defined input number
    for (
        max_firmware_in = 128;
        (max_firmware_in > 0) && !InputNumber::booked(max_firmware_in - 1);
        max_firmware_in--)
        ;

    if (computeOptimal)
    {
        // Automatically assign a default map
        for (uint8_t i = 0; i < 128; i++)
            if (InputNumber::booked(i))
            {
                // Do not overwrite other custom settings
                bool found = false;
                for (auto defMap : defaultMap)
                    if (defMap.firmware == i)
                    {
                        found = true;
                        break;
                    }
                uint8_t optimal = (max_firmware_in + i) % 128;
                if (!found)
                    ::inputMap::set(i, i, optimal);
            }
    }

    // Reset and load from flash memory
    InputMapService::call().resetMap();
    LoadSetting(UserSetting::INPUT_MAP);
}

//-------------------------------------------------------------------

void internals::inputMap::clear()
{
    for (uint8_t i = 0; i < 128; i++)
    {
        mapNoAlt[i] = i;
        mapAlt[i] = (i + 64) % 128;
    }
    defaultMap.clear();
}

//-------------------------------------------------------------------

void internals::inputMap::getReady()
{
    InputMapService::inject(new InputMapServiceProvider());
    OnStart.subscribe(inputMapStart);
}

//-------------------------------------------------------------------
// Map
//-------------------------------------------------------------------

void internals::inputMap::map(bool isAltModeEngaged, uint128_t &bitmap)
{
    uint128_t firmware_bitmap = bitmap;
    bitmap.low = 0ULL;
    bitmap.high = 0ULL;
    for (uint8_t i = 0; i < max_firmware_in; i++)
        if (firmware_bitmap.bit(i))
        {
            uint8_t user_input_number =
                (isAltModeEngaged) ? mapAlt[i] : mapNoAlt[i];
            bitmap.set_bit(user_input_number, true);
        }
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Public API
//-------------------------------------------------------------------
//-------------------------------------------------------------------

void inputMap::set(
    InputNumber firmware_defined,
    InputNumber user_defined,
    InputNumber user_defined_alt_engaged)
{
    if ((firmware_defined == UNSPECIFIED::VALUE) ||
        (user_defined == UNSPECIFIED::VALUE) ||
        (user_defined_alt_engaged == UNSPECIFIED::VALUE))
        throw unknown_input_number("Input map");

    DefaultMap defMap;
    defMap.firmware = firmware_defined;
    defMap.noAlt = user_defined;
    defMap.alt = user_defined_alt_engaged;
    defaultMap.push_back(defMap);
}

void inputMap::setOptimal()
{
    computeOptimal = true;
}
