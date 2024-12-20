/**
 * @file ShutdownTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-12-20
 * @brief Integration test. See [Readme](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <HardwareSerial.h>
#include "SimWheel.h"
#include "SimWheelTypes.h"
#include "debugUtils.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

class TestUI : public AbstractUserInterface
{
public:
    virtual void shutdown() override
    {
        Serial.printf("Device %u: shutdown.\n", _deviceID);
    }

    TestUI(uint8_t deviceID)
    {
        _deviceID = deviceID;
    }

private:
    uint8_t _deviceID;
};

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
    pixels::setAll(GRP_TELEMETRY, 85, 85, 85);
    pixels::show();

    auto ui1 = new TestUI(1);
    auto ui2 = new TestUI(2);
    notify::begin({ui1, ui2}, 0);

    Serial.println("-- GO --");
}

void loop()
{
    int chr = Serial.read();
    if (chr >= 0)
    {
        Serial.println("Command received: shutting down");
        power::powerOff();
    }
}