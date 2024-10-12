/**
 * @file PCF8574RevLightsTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-10-09
 * @brief Integration test for a user interface implementation.
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.h"
#include "SimWheelUI.h"
#include "debugUtils.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

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

    auto ui = new PCF8574RevLights(0);
    notify::begin({ui}, 50);
    userSettings::cpWorkingMode = CF_CLUTCH;
    userSettings::altButtonsWorkingMode = true;
    userSettings::dpadWorkingMode = true;
    userSettings::bitePoint = CLUTCH_DEFAULT_VALUE;
    userSettings::securityLock = false;
    hidImplementation::begin("PCF8574RevLights", "Mamandurrio", false);

    Serial.println("-- GO --");
}

void loop()
{
    delay(5000);
    hidImplementation::reset();
}