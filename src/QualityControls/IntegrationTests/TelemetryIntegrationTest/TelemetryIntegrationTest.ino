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
        Serial.printf(
            "Frame: %u Speed: %u. Pit limiter: %u. Blue flag: %u. Water temp: %u\n",
            data->frameID,
            data->powertrain.speed,
            data->ecu.pitLimiter,
            data->raceControl.blueFlag,
            data->gauges.waterTemperature);
    }
}

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

    notify::begin({new TelemetryDisplayMock()}, 10,3*1024);
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
    delay(1000);
    Serial.println("(Alive)");
    hidImplementation::reset();
}