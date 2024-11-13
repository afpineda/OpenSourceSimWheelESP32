/**
 * @file RevLightsSegmentTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-10-25
 * @brief Integration test for a user interface implementation.
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.h"
#include "SimWheelUI.h"
#include "debugUtils.h"
#include <HardwareSerial.h>

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

#define LED_COUNT 8
#define USE_LEVEL_SHIFT false
const revLightsMode_t displayMode = LEFT_TO_RIGHT;

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

void inputs::recalibrateAxes() {}

void inputs::reverseLeftAxis() {}

void inputs::reverseRightAxis() {}

void inputs::update() {}

void batteryCalibration::restartAutoCalibration() {}

int batteryMonitor::getLastBatteryLevel() { return 66; }

void power::powerOff() {}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    Serial.println("-- READY --");

    // Initialize
    auto ui = new LEDStripTelemetry(
        TEST_D_OUT,
        LED_COUNT,
        USE_LEVEL_SHIFT);
    new RevLightsLEDSegment(
        ui,
        0,
        LED_COUNT,
        0x00FF00,
        0xFFFF00,
        0xFF0000,
        0xFFFFFF,
        displayMode);
    notify::begin({ui}, 50);
    if (!capabilities::hasFlag(deviceCapability_t::CAP_TELEMETRY_POWERTRAIN))
        log_e("LEDStripTelemetry did not set the powertrain telemetry flag");

    userSettings::cpWorkingMode = CF_CLUTCH;
    userSettings::altButtonsWorkingMode = true;
    userSettings::dpadWorkingMode = true;
    userSettings::bitePoint = CLUTCH_DEFAULT_VALUE;
    userSettings::securityLock = false;
    capabilities::setFlag(deviceCapability_t::CAP_CLUTCH_BUTTON);
    hidImplementation::begin("RevLightsSegment", "Mamandurrio", false);

    Serial.println("-- GO --");
}

void loop()
{
    delay(5000);
    hidImplementation::reset();
}