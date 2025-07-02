/**
 * @file TelemetryIntegrationTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-03-11
 * @brief Integration test. See [Readme](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "Testing.hpp"
#include "SimWheel.hpp"
#include "SimWheelUI.hpp"
#include "SimWheelInternals.hpp"
#include "HAL.hpp"
#include "InternalServices.hpp"

#include <exception>

#include <HardwareSerial.h>

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

// Uncomment to test notifications involving battery SoC

class BatteryServiceMock: public BatteryService
{
public:
    virtual int getLastBatteryLevel() override { return 25; }
    virtual bool hasBattery() override { return true; }
};

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    try
    {
        Serial.println("-- READY --");

        // Configure I2C address
        internals::hal::i2c::initialize(
            TEST_SECONDARY_SDA,
            TEST_SECONDARY_SCL,
            I2CBus::SECONDARY);
        uint8_t hwAddress = 2;
        uint8_t factoryAddress = 0x38;
        Serial.printf("Using PCF8574 address %x (hexadecimal)\n", factoryAddress | hwAddress);

        hid::configure("UITest", "Mamandurrio", false);
        ui::add<SimpleShiftLight>(TEST_SIMPLE_SHIFT_LIGHT_PIN);
        ui::add<PCF8574RevLights>(
            hwAddress,
            I2CBus::SECONDARY,
            factoryAddress,
            RevLightsMode::LEFT_TO_RIGHT);

        internals::hid::common::getReady();
        internals::ui::getReady();

        // Uncomment to test notifications involving battery SoC
        BatteryService::inject(new BatteryServiceMock());

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
}