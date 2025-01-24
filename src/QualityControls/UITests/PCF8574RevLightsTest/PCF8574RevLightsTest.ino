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
#include "i2cTools.h"
#include <HardwareSerial.h>

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

revLightsMode_t mode = revLightsMode_t::LEFT_TO_RIGHT;

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

void inputs::recalibrateAxes() {}

void inputs::reverseLeftAxis() {}

void inputs::reverseRightAxis() {}

void inputs::update() {}

void inputs::setRotaryPulseX1() {}

void inputs::setRotaryPulseX2() {}

void inputs::setRotaryPulseX3() {}

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

    // Device discovery in secondary bus
    // Just to make sure we are using a correct I2C address
    i2c::begin(
        TEST_SECONDARY_SDA,
        TEST_SECONDARY_SCL,
        true);
    Serial.printf(
        "SDA = #%d. SCL = #%d. Auto-discovery in progress ...\n\n",
        TEST_SECONDARY_SDA,
        TEST_SECONDARY_SCL);
    std::vector<uint8_t> addressList;
    i2c::probe(addressList, true);
    size_t count = addressList.size();
    Serial.printf("Auto-discovery finished. %d device(s) found:\n", count);
    for (int idx = 0; idx < count; idx++)
    {
        uint8_t addr = addressList.at(idx);
        Serial.printf("- Device found at address %x (hexadecimal), %d (decimal)\n", addr, addr);
    }

    // Configure I2C address
    uint8_t hwAddress = 0;
    uint8_t factoryAddress = 0x38;
    Serial.printf("Using device address %x (hexadecimal)\n", factoryAddress | hwAddress);

    // Initialize UI
    auto ui = new PCF8574RevLights(hwAddress, true, factoryAddress, mode);
    notify::begin({ui}, 50);

    // Check
    if (!capabilities::hasFlag(deviceCapability_t::CAP_TELEMETRY_POWERTRAIN))
        log_e("PCF8574RevLights did not set the powertrain telemetry flag");

    // Initialize other namespaces
    userSettings::cpWorkingMode = CF_CLUTCH;
    userSettings::altButtonsWorkingMode = true;
    userSettings::dpadWorkingMode = true;
    userSettings::bitePoint = CLUTCH_DEFAULT_VALUE;
    userSettings::securityLock = false;
    capabilities::setFlag(deviceCapability_t::CAP_CLUTCH_BUTTON);
    hidImplementation::begin("PCF8574RevLights", "Mamandurrio", false);

    Serial.println("-- GO --");
}

void loop()
{
    delay(5000);
    hidImplementation::reset();
}