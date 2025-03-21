/**
 * @file inputMap.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Translates firmware-defined input numbers to user-defined input numbers.
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

static std::array<uint8_t, 64> mapNoAlt;
static std::array<uint8_t, 64> mapAlt;
static std::vector<DefaultMap> defaultMap;
static bool computeOptimal = false;

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
        if ((firmware_defined < 64) && (user_defined < 128) && (user_defined_alt < 128))
        {
            mapNoAlt[firmware_defined] = user_defined;
            mapAlt[firmware_defined] = user_defined_alt;
            // SaveSetting::notify(UserSetting::INPUT_MAP);
        }
    }

    virtual void getMap(
        uint8_t firmware_defined,
        uint8_t &user_defined,
        uint8_t &user_defined_alt) override
    {
        if (firmware_defined < 64)
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
        for (uint8_t i = 0; i < 64; i++)
        {
            mapNoAlt[i] = i;
            mapAlt[i] = (i + 64);
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
    InputMapService::call::resetMap();
    LoadSetting::notify(UserSetting::INPUT_MAP);
}

//-------------------------------------------------------------------

void internals::inputMap::clear()
{
    for (uint8_t i = 0; i < 64; i++)
    {
        mapNoAlt[i] = i;
        mapAlt[i] = (i + 64);
    }
    defaultMap.clear();
}

//-------------------------------------------------------------------

void internals::inputMap::getReady()
{
    if (computeOptimal)
    {
        // Compute the highest firmware-defined input number
        uint8_t max_firmware_in;
        for (
            max_firmware_in = 64;
            (max_firmware_in > 0) && !InputNumber::booked(max_firmware_in - 1);
            max_firmware_in--)
            ;
        // Automatically assign a new map
        for (uint8_t i = 0; i < 64; i++)
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
                if (!found)
                    ::inputMap::set(i, i, i + max_firmware_in);
            }
    }
    for (auto defMap : defaultMap)
        if (!InputNumber::booked(defMap.firmware))
            throw std::runtime_error(
                "The input number " +
                std::to_string(defMap.firmware) +
                " can not be mapped, since it is not assigned");
    InputMapService::inject(new InputMapServiceProvider());
    OnStart::subscribe(inputMapStart);
}

//-------------------------------------------------------------------
// Map
//-------------------------------------------------------------------

void internals::inputMap::map(
    bool isAltModeEngaged,
    uint64_t firmware_bitmap,
    uint64_t &low,
    uint64_t &high)
{
    high = 0ULL;
    low = 0ULL;
    for (uint8_t i = 0; i < 64; i++)
        if (firmware_bitmap & (1ULL << i))
        {
            uint8_t user_input_number = (isAltModeEngaged) ? mapAlt[i] : mapNoAlt[i];
            if (user_input_number < 64)
                low |= (1ULL << user_input_number);
            else
                high |= (1ULL << (user_input_number - 64));
        }
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Public API
//-------------------------------------------------------------------
//-------------------------------------------------------------------

void inputMap::set(
    InputNumber firmware_defined,
    UserInputNumber user_defined,
    UserInputNumber user_defined_alt_engaged)
{
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
