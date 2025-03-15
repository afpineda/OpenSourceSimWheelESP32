/**
 * @file firmware.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-27
 * @brief Performs initialization and launches execution.
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

#if !CD_CI

#include <exception>
#include "HardwareSerial.h"
#include "freertos/FreeRTOS.h"

#endif

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Internal
//-------------------------------------------------------------------
//-------------------------------------------------------------------

void firmwareSetIsRunningState(bool state)
{
    FirmwareService::_is_running = state;
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Public
//-------------------------------------------------------------------
//-------------------------------------------------------------------

void firmware::run()
{
    if (!FirmwareService::call::isRunning())
    {
        internals::storage::getReady();
        internals::hid::common::getReady();
        internals::inputMap::getReady();
        internals::inputHub::getReady();
        internals::inputs::getReady();
        internals::batteryCalibration::getReady();
        internals::batteryMonitor::getReady();
        internals::power::getReady();
        internals::pixels::getReady();
        internals::ui::getReady();
        OnStart::notify();
        firmwareSetIsRunningState(true);
    }
}

void firmware::run(void (*func)())
{
#if CD_CI
    func();
    firmware::run();
#else
    // Arduino-ESP32 does not print exception messages
    try
    {
        func();
        firmware::run();
    }
    catch (std::exception &e)
    {
        Serial.end();
        Serial.begin(115200);
        for (;;)
        {
            Serial.println("**CUSTOM FIRMWARE ERROR**");
            Serial.println(e.what());
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }
#endif
}
