/**
 * @file TelemetryIntegrationTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-10-02
 * @brief Integration test. See [Readme](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <HardwareSerial.h>
#include "SimWheel.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

class TelemetryDisplayMock : public AbstractUserInterface
{
public:
    TelemetryDisplayMock();
    virtual void onTelemetryData(const telemetryData_t *data) override;
};

TelemetryDisplayMock::TelemetryDisplayMock()
{
    requiresPowertrainTelemetry = true;
    requiresECUTelemetry = true;
    requiresRaceControlTelemetry = true;
    requiresGaugeTelemetry = true;
}

void TelemetryDisplayMock::onTelemetryData(const telemetryData_t *data)
{
    if (data == nullptr)
    {
        Serial.println("(No telemetry)");
    }
    else
    {
        Serial.println("*******************");
        Serial.printf("* Frame ID: %lu\n", data->frameID);
        Serial.println("*******************");
        Serial.printf(
            "  Gear: %c RPM: %u / %u%% SL1: %u SL2: %u RevLim: %u Speed: %u Running: %u\n",
            data->powertrain.gear,
            data->powertrain.rpm,
            data->powertrain.rpmPercent,
            data->powertrain.shiftLight1,
            data->powertrain.shiftLight2,
            data->powertrain.revLimiter,
            data->powertrain.speed,
            data->powertrain.engineStarted);
        Serial.printf(
            "  Abs: %u TC: %u DRS: %u PitLim: %u LowFuel: %u ABSLevel: %u TCLevel: %u, BrakeBias: %u%%\n",
            data->ecu.absEngaged,
            data->ecu.tcEngaged,
            data->ecu.drsEngaged,
            data->ecu.pitLimiter,
            data->ecu.lowFuelAlert,
            data->ecu.absLevel,
            data->ecu.tcLevel,
            data->ecu.brakeBias);
        Serial.printf(
            "  BlueF: %u YellowF: %u GreenF: %u Laps rem.: %u Minutes rem.: %u\n",
            data->raceControl.blueFlag,
            data->raceControl.yellowFlag,
            data->raceControl.greenFlag,
            data->raceControl.remainingLaps,
            data->raceControl.remainingMinutes);
        Serial.printf(
            "  WaterTemp: %u OilTemp: %u OilPressure: %f AbsFuel: %u RelFuel: %u\n",
            data->gauges.waterTemperature,
            data->gauges.oilTemperature,
            data->gauges.oilPressure,
            data->gauges.absoluteRemainingFuel,
            data->gauges.relativeRemainingFuel);
    }
}

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

void inputs::recalibrateAxes() {}

void inputs::reverseLeftAxis() {}

void inputs::reverseRightAxis() {}

void inputs::update() {}

void inputs::setRotaryPulseWidthMultiplier(uint8_t multiplier) {}

uint8_t inputs::getRotaryPulseWidthMultiplier() { return 1; }

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

    notify::begin({new TelemetryDisplayMock()}, 3, 3 * 1024);
    userSettings::cpWorkingMode = CF_CLUTCH;
    userSettings::altButtonsWorkingMode = true;
    userSettings::dpadWorkingMode = true;
    userSettings::bitePoint = CLUTCH_DEFAULT_VALUE;
    userSettings::securityLock = false;
    hidImplementation::begin("TelemIntTest", "Mamandurrio", false);

    Serial.println("-- GO --");
}

void loop()
{
    delay(5000);
    hidImplementation::reset();
}