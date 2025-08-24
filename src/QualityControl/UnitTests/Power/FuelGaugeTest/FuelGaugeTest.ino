/**
 * @file FuelGaugeTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-08-19
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "Testing.hpp"
#include "BatteryMonitorHardware.hpp"
#include "InternalServices.hpp"
#include "HAL.hpp"
#include <optional>

#include <HardwareSerial.h>

//-------------------------------------------------------
// Globals
//-------------------------------------------------------

MAX1704x *hw;

//-------------------------------------------------------
// Mocks
//-------------------------------------------------------

class BatteryCalibrationMock : public BatteryCalibrationService
{
public:
    virtual int getBatteryLevel(int reading) override
    {
        return reading / 41; // 0-4096 <=> 0-99 linear
    }
} calMock;

//-------------------------------------------------------
// Auxiliary
//-------------------------------------------------------

void printStatusBool(const std::string header, const std::optional<bool> &opt)
{
    Serial.print(header.c_str());
    Serial.print(": ");
    if (opt.has_value())
        Serial.printf("%s", opt.value() ? "true" : "false");
    else
        Serial.print("unknown");
    Serial.println("");
}

void printStatusUint8(const std::string header, const std::optional<uint8_t> &opt)
{
    Serial.print(header.c_str());
    Serial.print(": ");
    if (opt.has_value())
        Serial.printf("%u", opt.value());
    else
        Serial.print("unknown");
    Serial.println("");
}

//-------------------------------------------------------
// Entry point
//-------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    Serial.println("--READY--");
    BatteryCalibrationService::inject(&calMock);

    hw = new MAX1704x();

    Serial.println("--GO--");
}

void loop()
{
    Serial.println("Getting battery status...");
    BatteryStatus status;
    hw->getStatus(status);

    printStatusBool("Battery presence", status.isBatteryPresent);
    printStatusBool("Charging", status.isCharging);
    printStatusBool("Wired power", status.usingExternalPower);
    printStatusUint8("SoC", status.stateOfCharge);

    Serial.println("Done.");
    DELAY_MS(5000);
}
