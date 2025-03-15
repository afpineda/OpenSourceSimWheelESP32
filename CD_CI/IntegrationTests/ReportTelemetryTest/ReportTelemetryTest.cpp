/**
 * @file ReportTelemetryTest.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-03-03
 * @brief Integration test
 *
 * @copyright Licensed under the EUPL
 *
 */

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include "HID_definitions.hpp"
#include "cd_ci_assertions.hpp"

#include <cinttypes>
#include <cassert>
#include <iostream>

//-------------------------------------------------------------------
// Auxiliary
//-------------------------------------------------------------------

uint8_t report20[] = {0x4b, 0x5e, 0x06, 0x63, 0x01, 0x00, 0x01, 0x00, 0x40, 0x01};
uint8_t report21[] = {0x00, 0x02, 0xff, 0x00, 0x01, 0x00, 0x07, 0x40, 0xfe};
uint8_t report22[] = {0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0xe2, 0x09, 0x6c, 0x02};
uint8_t report23[] = {0x56, 0x71, 0x00, 0xb0, 0x04, 0x5e, 0x27, 0xbf, 0x04, 0x01, 0x74, 0x00};

#define BYTES(s) ((uint8_t *)&s)
#define AS_UINT16(s) *((uint16_t *)(s))

//-------------------------------------------------------------------
// Mocks
//-------------------------------------------------------------------

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------
//-------------------------------------------------------------------

using namespace telemetry;

int main()
{
    assert((sizeof(report20) == POWERTRAIN_REPORT_SIZE) && "Test is outdated");
    assert((sizeof(report21) == ECU_REPORT_SIZE) && "Test is outdated");
    assert((sizeof(report22) == RACE_CONTROL_REPORT_SIZE) && "Test is outdated");
    assert((sizeof(report23) == GAUGES_REPORT_SIZE) && "Test is outdated");

    // Report 20
    std::cout << "- Report 20 (powertrain) -" << std::endl;
    internals::hid::common::onOutput(RID_OUTPUT_POWERTRAIN, report20, sizeof(report20));
    assert(data.powertrain.gear == 0x4B);
    assert(data.powertrain.rpm == 1630);
    assert(data.powertrain.rpmPercent == 99);
    assert(data.powertrain.shiftLight1 == 1);
    assert(data.powertrain.shiftLight2 == 0);
    assert(data.powertrain.revLimiter == true);
    assert(data.powertrain.engineStarted == false);
    assert(data.powertrain.speed == 320);

    // Report 21
    std::cout << "- Report 21 (ecu) -" << std::endl;
    internals::hid::common::onOutput(RID_OUTPUT_ECU, report21, sizeof(report21));
    assert(data.ecu.absEngaged == false);
    assert(data.ecu.tcEngaged == true);
    assert(data.ecu.drsEngaged == true);
    assert(data.ecu.pitLimiter == false);
    assert(data.ecu.lowFuelAlert == true);
    assert(data.ecu.absLevel == false);
    assert(data.ecu.tcLevel == 7);
    assert(data.ecu.tcCut == 64);
    assert(data.ecu.brakeBias == 100);

    // Report 22
    std::cout << "- Report 22 (race control) -" << std::endl;
    internals::hid::common::onOutput(RID_OUTPUT_RACE_CONTROL, report22, sizeof(report22));
    assert(data.raceControl.blackFlag == true);
    assert(data.raceControl.blueFlag == false);
    assert(data.raceControl.checkeredFlag == true);
    assert(data.raceControl.greenFlag == false);
    assert(data.raceControl.orangeFlag == true);
    assert(data.raceControl.whiteFlag == false);
    assert(data.raceControl.yellowFlag == true);
    assert(data.raceControl.remainingLaps == 2530);
    assert(data.raceControl.remainingMinutes == 620);

    // Report 23
    std::cout << "- Report 23 (gauges) -" << std::endl;
    internals::hid::common::onOutput(RID_OUTPUT_GAUGES, report23, sizeof(report23));
    assert(data.gauges.relativeTurboPressure == 86);
    assert<float>::almostEquals("Turbo pressure", 1.13, data.gauges.absoluteTurboPressure, 0.01);
    assert(data.gauges.waterTemperature == 1200);
    assert<float>::almostEquals("Oil pressure", 100.78, data.gauges.oilPressure, 0.01);
    assert(data.gauges.oilTemperature == 1215);
    assert(data.gauges.relativeRemainingFuel == 1);
    assert(data.gauges.absoluteRemainingFuel == 116);
}
