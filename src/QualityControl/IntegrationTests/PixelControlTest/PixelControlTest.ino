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
#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "Testing.hpp"
#include "InternalServices.hpp"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

// Uncomment to test notifications involving battery SoC

// class BatteryServiceMock: public BatteryService
// {
// public:
//     virtual int getLastBatteryLevel() override { return 25; }
//     virtual bool hasBattery() override { return true; }
// };

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    try
    {
        Serial.println("-- READY --");
        pixels::configure(
            PixelGroup::GRP_TELEMETRY,
            TEST_D_OUT,
            8,
            TEST_LEVEL_SHIFTER,
            PixelDriver::WS2812,
            PixelFormat::AUTO,
            16);
        ui::addPixelControlNotifications();
        hid::configure("PixelCtrlTest", "Mamandurrio", false);

        internals::pixels::getReady();
        internals::hid::common::getReady();
        internals::ui::getReady();

        // Uncomment to test notifications involving battery SoC
        // BatteryService::inject(new BatteryServiceMock());

        OnStart::notify();

        Serial.println("-- GO --");
    }
    catch (std::exception &e)
    {
        Serial.println("EXCEPTION:");
        Serial.println(e.what());
        for (;;)
            ;
    }
}

void loop()
{
    int chr = Serial.read();
    if (chr >= 0)
    {
        if ((chr == 's') || (chr == 'S'))
        {
            Serial.println("SHUTDOWN");
            OnShutdown::notify();
        }
        else
        {
            OnLowBattery::notify();
            OnBitePoint::notify(CLUTCH_DEFAULT_VALUE);
            OnSettingsSaved::notify();
        }
    }
    delay(250);
}