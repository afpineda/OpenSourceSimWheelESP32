/**
 * @file NimBLEimplTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <HardwareSerial.h>
#include "Simwheel.h"
#include "HID_definitions.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

bool powerSim = true;
extern uint16_t customVID;
extern uint16_t customPID;
extern uint16_t factoryVID;
extern uint16_t factoryPID;

uint64_t lastFrameID = 0;

//------------------------------------------------------------------
// mocks
//------------------------------------------------------------------

volatile uint32_t capabilities::flags = 0x07;
volatile inputBitmap_t capabilities::availableInputs = 0b0111ULL;
volatile telemetryData_t notify::telemetryData = {};
uint8_t notify::maxFPS = 0;

void notify::connected()
{
    Serial.println("*** CONNECTED ***");
}

void notify::BLEdiscovering()
{
    Serial.println("*** DISCOVERING ***");
}

void notify::bitePoint()
{
}

void notify::selectNextPage(uint8_t index)
{
    Serial.println("UI control: next page");
}

void notify::selectPreviousPage(uint8_t index)
{
    Serial.println("UI control: previous page");
}

uint8_t notify::getUICount()
{
    return 1;
}

void inputs::recalibrateAxes()
{
    Serial.println("CMD: recalibrate axes");
}

void inputs::update()
{
}

void inputs::reverseLeftAxis()
{
    Serial.println("CMD: reverse left axis");
}

void inputs::reverseRightAxis()
{
    Serial.println("CMD: reverse right axis");
}

void batteryCalibration::restartAutoCalibration()
{
    Serial.println("CMD: recalibrate battery");
}

void power::powerOff()
{
    Serial.println("*** POWER OFF ***");
    powerSim = false;
}

int batteryMonitor::getLastBatteryLevel()
{
    return UNKNOWN_BATTERY_LEVEL;
}

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

void checkAndPrintTelemetryData()
{
    if (notify::telemetryData.frameID != lastFrameID)
    {
        lastFrameID = notify::telemetryData.frameID;
        Serial.printf("powertrain: %c %u %u %u %u %u\n",
                      notify::telemetryData.powertrain.gear,
                      notify::telemetryData.powertrain.rpm,
                      notify::telemetryData.powertrain.rpmPercent,
                      notify::telemetryData.powertrain.shiftLight1,
                      notify::telemetryData.powertrain.shiftLight2,
                      notify::telemetryData.powertrain.speed);
        Serial.printf("ecu: %u %u %u %u %u %u %u %u %u\n",
                      notify::telemetryData.ecu.absEngaged,
                      notify::telemetryData.ecu.tcEngaged,
                      notify::telemetryData.ecu.drsEngaged,
                      notify::telemetryData.ecu.pitLimiter,
                      notify::telemetryData.ecu.lowFuelAlert,
                      notify::telemetryData.ecu.absLevel,
                      notify::telemetryData.ecu.tcLevel,
                      notify::telemetryData.ecu.tcCut,
                      notify::telemetryData.ecu.brakeBias);
        Serial.printf("race control: %u %u %u %u %u %u %u %u %u\n",
                      notify::telemetryData.raceControl.blackFlag,
                      notify::telemetryData.raceControl.blueFlag,
                      notify::telemetryData.raceControl.checkeredFlag,
                      notify::telemetryData.raceControl.greenFlag,
                      notify::telemetryData.raceControl.orangeFlag,
                      notify::telemetryData.raceControl.whiteFlag,
                      notify::telemetryData.raceControl.yellowFlag,
                      notify::telemetryData.raceControl.remainingLaps,
                      notify::telemetryData.raceControl.remainingMinutes);
        Serial.printf("gauges: %u %.2f %u %.2f %u %u %u %u %u\n",
                      notify::telemetryData.gauges.relativeTurboPressure,
                      notify::telemetryData.gauges.absoluteTurboPressure,
                      notify::telemetryData.gauges.waterTemperature,
                      notify::telemetryData.gauges.oilPressure,
                      notify::telemetryData.gauges.oilTemperature,
                      notify::telemetryData.gauges.relativeRemainingFuel,
                      notify::telemetryData.gauges.absoluteRemainingFuel,
                      notify::telemetryData.gauges.remainingFuelLaps,
                      notify::telemetryData.gauges.remainingFuelMinutes);
    }
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    Serial.println("--START--");
    userSettings::altButtonsWorkingMode = true;
    userSettings::cpWorkingMode = CF_CLUTCH;
    userSettings::dpadWorkingMode = true;
    userSettings::bitePoint = CLUTCH_DEFAULT_VALUE;
    userSettings::securityLock = false;
    hidImplementation::begin("NimBLEimplTest", "Mamandurrio", true);
    Serial.printf("Factory default VID / PID: %04x / %04x\n", factoryVID, factoryPID);
    Serial.printf("Actual VID / PID: %04x / %04x\n", customVID, customPID);
    Serial.println("--GO--");
}

//------------------------------------------------------------------

uint8_t btnIndex = 0;
clutchValue_t axis = CLUTCH_NONE_VALUE;
uint8_t battery = 99;
uint8_t POV = 0;

void loop()
{
    if (!powerSim)
    {
        // Simulate power off
        Serial.println("(Reset required)");
        for (;;)
            ;
    }

    if (!hidImplementation::isConnected())
    {
        Serial.println("(Waiting for connection)");
    }
    else
    {
        inputBitmap_t data = BITMAP(btnIndex);
        hidImplementation::reportInput(
            data,
            data,
            POV,
            axis,
            axis,
            axis);

        // Update pressed buttons
        btnIndex++;
        if (btnIndex > MAX_INPUT_NUMBER)
            btnIndex = 0;

        // Update DPAD state
        POV = POV + 1;
        if (POV > 8)
        {
            POV = 0;
            hidImplementation::reportChangeInConfig();
        }

        // Update battery info
        battery--;
        if (battery < 50)
            battery = 100;
        hidImplementation::reportBatteryLevel(battery);

        // Update analog axis values
        axis = axis + 5;
        if (axis >= CLUTCH_FULL_VALUE - 5)
            axis = CLUTCH_NONE_VALUE;

        // Print telemetry data (if any)
        checkAndPrintTelemetryData();
    }

    // Wait a second
    delay(1000);
}