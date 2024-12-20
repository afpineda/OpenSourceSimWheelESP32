/**
 * @file PixelControlTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-12-13
 * @brief Integration test. See [Readme](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <HardwareSerial.h>
#include "SimWheel.h"
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

    pixels::configure(
        GRP_TELEMETRY,
        TEST_D_OUT,
        8,
        TEST_LEVEL_SHIFTER,
        pixel_driver_t::WS2812,
        pixel_format_t::AUTO,
        16);
    notify::begin({new PixelControlNotification()});
    userSettings::cpWorkingMode = CF_CLUTCH;
    userSettings::altButtonsWorkingMode = true;
    userSettings::dpadWorkingMode = true;
    userSettings::bitePoint = CLUTCH_DEFAULT_VALUE;
    userSettings::securityLock = false;
    hidImplementation::begin("PixelCtrlTest", "Mamandurrio", false);

    Serial.println("-- GO --");
}

void loop()
{
    int chr = Serial.read();
    if (chr >= 0)
    {
        if ((chr == 's') || (chr == 'S'))
        {
            Serial.println("SHUTDOWN");
            pixels::shutdown();
        }
        else
        {
            notify::lowBattery();
            notify::bitePoint();
        }
    }
    delay(250);
}