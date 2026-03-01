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
    debugPrintf("%s: ", header.c_str());
    if (opt.has_value())
        debugPrintf("%s", opt.value() ? "true" : "false");
    else
        debugPrintf("unknown");
    debugPrintf("\n");
}

void printStatusUint8(const std::string header, const std::optional<uint8_t> &opt)
{
    debugPrintf("%s: ", header.c_str());
    if (opt.has_value())
        debugPrintf("%u", opt.value());
    else
        debugPrintf("unknown");
    debugPrintf("\n");
}

//-------------------------------------------------------
// Entry point
//-------------------------------------------------------

void setup()
{
    debugPrintBegin();
    debugPrintf("--READY--\n");
    BatteryCalibrationService::inject(&calMock);

    hw = new MAX1704x();

    debugPrintf("--GO--\n");
}

void loop()
{
    debugPrintf("Getting battery status...\n");
    BatteryStatus status;
    hw->getStatus(status);

    printStatusBool("Battery presence", status.isBatteryPresent);
    printStatusBool("Charging", status.isCharging);
    printStatusBool("Wired power", status.usingExternalPower);
    printStatusUint8("SoC", status.stateOfCharge);

    debugPrintf("Done.\n");
    DELAY_MS(5000);
}
