/**
 * @file SinglePixelSegmentsTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-10-26
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

    auto ui = new LEDStripTelemetry(
        TEST_D_OUT,
        LED_COUNT,
        USE_LEVEL_SHIFT);
    ui->brightness(31);
    new ShiftLightLEDSegment(ui, 0);
    new RaceFlagsLEDSegment(ui, 2, 250);
    new RaceFlagsLEDSegment(ui, 3, 0);
    new WitnessLEDSegment(ui, 5,
                          witness_t::ABS_ENGAGED, 0xFF0000);
    new WitnessLEDSegment(ui, 6,
                          witness_t::TC_ENGAGED, 0x00FF00,
                          witness_t::ABS_ENGAGED, 0xFF0000);
    new WitnessLEDSegment(ui, 7,
                          witness_t::PIT_LIMITER, 0xFFFF00,
                          witness_t::ABS_ENGAGED, 0xFF0000,
                          witness_t::TC_ENGAGED, 0x00FF00);

    notify::begin({ui}, 50);
    if (!capabilities::hasFlag(deviceCapability_t::CAP_TELEMETRY_POWERTRAIN))
        log_e("LEDStripTelemetry did not set the powertrain telemetry flag");
    if (!capabilities::hasFlag(deviceCapability_t::CAP_TELEMETRY_RACE_CONTROL))
        log_e("LEDStripTelemetry did not set the race control telemetry flag");
    if (!capabilities::hasFlag(deviceCapability_t::CAP_TELEMETRY_ECU))
        log_e("LEDStripTelemetry did not set the ecu telemetry flag");

    userSettings::cpWorkingMode = CF_CLUTCH;
    userSettings::altButtonsWorkingMode = true;
    userSettings::dpadWorkingMode = true;
    userSettings::bitePoint = CLUTCH_DEFAULT_VALUE;
    userSettings::securityLock = false;
    hidImplementation::begin("SinglePixelSegments", "Mamandurrio", false);

    Serial.println("-- GO --");
}

void loop()
{
    delay(5000);
    hidImplementation::reset();
}