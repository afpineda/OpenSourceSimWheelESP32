/**
 * @file hidTestHelper.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-09-21
 *
 * @brief Helper code for the testing of HID implementations.
 *
 * @copyright Licensed under the EUPL
 *
 */

#ifdef HID_TESTER

#include <HardwareSerial.h>
#include "Simwheel.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

bool powerSim = true;
extern uint16_t customVID;
extern uint16_t customPID;
extern uint16_t factoryVID;
extern uint16_t factoryPID;

uint32_t lastFrameID = 0;

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

volatile uint32_t capabilities::flags = 0x07;
volatile inputBitmap_t capabilities::availableInputs = 0b0111ULL;
volatile telemetryData_t notify::telemetryData = {};
uint8_t notify::maxFPS = 50;

//------------------------------------------------------------------

void notify::connected()
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    Serial.println("*** CONNECTED ***");
#endif
}

void notify::BLEdiscovering()
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    Serial.println("*** DISCOVERING ***");
#endif
}

void notify::bitePoint()
{
}

//------------------------------------------------------------------

void inputs::recalibrateAxes()
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    Serial.println("CMD: recalibrate axes");
#endif
}

void inputs::update()
{
}

void inputs::reverseLeftAxis()
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    Serial.println("CMD: reverse left axis");
#endif
}

void inputs::reverseRightAxis()
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    Serial.println("CMD: reverse right axis");
#endif
}

void inputs::setRotaryPulseX1()
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    Serial.println("CMD: pulse width x1");
#endif
}

void inputs::setRotaryPulseX2()
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    Serial.println("CMD: pulse width x2");
#endif
}

void inputs::setRotaryPulseX3()
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    Serial.println("CMD: pulse width x3");
#endif
}

//------------------------------------------------------------------

void batteryCalibration::restartAutoCalibration()
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    Serial.println("CMD: recalibrate battery");
#endif
}

//------------------------------------------------------------------

void power::powerOff()
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    Serial.println("*** POWER OFF ***");
#endif
    powerSim = false;
}

//------------------------------------------------------------------

int batteryMonitor::getLastBatteryLevel()
{
    return UNKNOWN_BATTERY_LEVEL;
}

//------------------------------------------------------------------

void pixels::set(
    pixelGroup_t group,
    uint8_t pixelIndex,
    uint8_t red,
    uint8_t green,
    uint8_t blue)
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    Serial.printf("pixels::set(%hhu,%hhu,%hhu,%hhu,%hhu)\n",
                  group,
                  pixelIndex,
                  red,
                  green,
                  blue);
#endif
}

void pixels::reset()
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    Serial.println("pixels::reset()");
#endif
}

void pixels::show()
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    Serial.println("pixels::show()");
#endif
}

uint8_t pixels::getPixelCount(pixelGroup_t group)
{
    return 8;
}

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

void checkAndPrintTelemetryData()
{
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    if (notify::telemetryData.frameID != lastFrameID)
    {
        lastFrameID = notify::telemetryData.frameID;
        Serial.printf("powertrain: %c %u %u %u %u %u %u %u\n",
                      notify::telemetryData.powertrain.gear,
                      notify::telemetryData.powertrain.rpm,
                      notify::telemetryData.powertrain.rpmPercent,
                      notify::telemetryData.powertrain.shiftLight1,
                      notify::telemetryData.powertrain.shiftLight2,
                      notify::telemetryData.powertrain.revLimiter,
                      notify::telemetryData.powertrain.engineStarted,
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
        Serial.printf("gauges: %u %.2f %u %.2f %u %u %u\n",
                      notify::telemetryData.gauges.relativeTurboPressure,
                      notify::telemetryData.gauges.absoluteTurboPressure,
                      notify::telemetryData.gauges.waterTemperature,
                      notify::telemetryData.gauges.oilPressure,
                      notify::telemetryData.gauges.oilTemperature,
                      notify::telemetryData.gauges.relativeRemainingFuel,
                      notify::telemetryData.gauges.absoluteRemainingFuel);
    }
#endif
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    Serial.begin(115200);
    Serial.println("--START--");
#endif
    userSettings::altButtonsWorkingMode = true;
    userSettings::cpWorkingMode = CF_CLUTCH;
    userSettings::dpadWorkingMode = true;
    userSettings::bitePoint = CLUTCH_DEFAULT_VALUE;
    userSettings::securityLock = false;
    hidImplementation::begin(HID_TESTER, "Mamandurrio", true);
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
    Serial.printf("Factory default VID / PID: %04x / %04x\n", factoryVID, factoryPID);
    Serial.printf("Actual VID / PID: %04x / %04x\n", customVID, customPID);
    Serial.println("--GO--");
#endif
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
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
        Serial.println("(Reset required)");
#endif
        for (;;)
            ;
    }

    if (!hidImplementation::isConnected())
    {
#if (ARDUINO_USB_MODE == 1) || defined(CONFIG_IDF_TARGET_ESP32)
        Serial.println("(Waiting for connection)");
#endif
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

#endif