/**
 * @file FirmwareRunTest.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-09-26
 * @brief Unit test
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include <cassert>
#include <iostream>

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

bool customFirmwareReady = false;
bool batteryMonitorReady = false;
bool batteryCalibrationReady = false;
bool pixelsReady = false;
bool uiReady = false;
bool inputsReady = false;
bool inputHubReady = false;
bool storageReady = false;
bool inputMapReady = false;
bool powerReady = false;
bool hidCommonReady = false;
bool started = false;

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

void customFirmware() { customFirmwareReady = true; }

void onStart() { started = true; }

void internals::batteryMonitor::getReady() { batteryMonitorReady = true; }

void internals::batteryCalibration::getReady() { batteryCalibrationReady = true; }

void internals::pixels::getReady() { pixelsReady = true; }

void internals::ui::getReady() { uiReady = true; }

void internals::inputs::getReady() { inputsReady = true; }

void internals::inputHub::getReady() { inputHubReady = true; }

void internals::storage::getReady() { storageReady = true; }

void internals::inputMap::getReady() { inputMapReady = true; }

void internals::power::getReady() { powerReady = true; }

void internals::hid::common::getReady() { hidCommonReady = true; }

//------------------------------------------------------------------
//------------------------------------------------------------------
// Entry point
//------------------------------------------------------------------
//------------------------------------------------------------------

int main()
{
    // Check that this test initialization is good
    assert(!customFirmwareReady);
    assert(!batteryMonitorReady);
    assert(!batteryCalibrationReady);
    assert(!pixelsReady);
    assert(!uiReady);
    assert(!inputsReady);
    assert(!inputHubReady);
    assert(!storageReady);
    assert(!inputMapReady);
    assert(!powerReady);
    assert(!hidCommonReady);
    assert(!started);

    // Check that the firmware is not running
    assert(!FirmwareService::call::isRunning());

    // Start subsystems
    OnStart::subscribe(onStart);
    firmware::run(customFirmware);

    // Check that all subsystems where initialized and started
    assert(customFirmwareReady);
    assert(batteryMonitorReady);
    assert(batteryCalibrationReady);
    assert(pixelsReady);
    assert(uiReady);
    assert(inputsReady);
    assert(inputHubReady);
    assert(storageReady);
    assert(inputMapReady);
    assert(powerReady);
    assert(hidCommonReady);
    assert(started);

    // Check that the firmware is running
    assert(FirmwareService::call::isRunning());

    return 0;
}