/**
 * @file InputHubTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-05
 * @brief Integration test. See [Readme](./README.md)
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "debugUtils.h"
#include "SimWheel.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

// Note: Do not assign an input number greater than 15 in this test
#define LEFT_CLUTCH_IN 0
#define RIGHT_CLUTCH_IN 1
#define COMMAND_IN 2
#define CYCLE_ALT_IN 3
#define CYCLE_CLUTCH_IN 4
#define ALT_IN 10
#define CW_IN 12
#define CCW_IN 13

clutchFunction_t oldCP;
bool oldAltF;

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

void hidImplementation::reportInput(
    inputBitmap_t inputsLow,
    inputBitmap_t inputsHigh,
    uint8_t POVstate,
    clutchValue_t leftAxis,
    clutchValue_t rightAxis,
    clutchValue_t clutchAxis)
{
    Serial.print("...");
    debugPrintBool(inputsHigh, 16);
    Serial.print(" ...");
    debugPrintBool(inputsLow, 16);
    Serial.printf(" | %3.3d | %3.3d | %3.3d || %3.3d\n",
                  leftAxis,
                  rightAxis,
                  clutchAxis,
                  userSettings::bitePoint);
}

void hidImplementation::reset()
{
    Serial.println("HID RESET");
}

void hidImplementation::reportChangeInConfig()
{
}

void notify::bitePoint(inputNumber_t b)
{
}

void batteryCalibration::restartAutoCalibration()
{
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    Serial.println("-- READY --");

    inputs::addButtonMatrix(
        mtxSelectors,
        sizeof(mtxSelectors) / sizeof(mtxSelectors[0]),
        mtxInputs,
        sizeof(mtxInputs) / sizeof(mtxInputs[0]),
        mtxNumbers);
    inputs::addDigital(TEST_ROTARY_SW, ALT_IN, true, true);
    inputs::addRotaryEncoder(TEST_ROTARY_CLK, TEST_ROTARY_DT, CW_IN, CCW_IN, false);
    inputs::setAnalogClutchPaddles(TEST_ANALOG_PIN1, TEST_ANALOG_PIN2);

    inputHub::setClutchInputNumbers(LEFT_CLUTCH_IN, RIGHT_CLUTCH_IN);
    inputHub::setALTButton(ALT_IN);
    inputHub::cycleALTButtonsWorkingMode_setBitmap(BITMAP(COMMAND_IN) | BITMAP(CYCLE_ALT_IN));
    inputHub::cycleCPWorkingMode_setBitmap(BITMAP(COMMAND_IN) | BITMAP(CYCLE_CLUTCH_IN));
    inputHub::setClutchCalibrationButtons(CW_IN, CCW_IN);

    oldCP = CF_BUTTON;
    userSettings::setCPWorkingMode(oldCP);
    oldAltF = true;
    userSettings::setALTButtonsWorkingMode(oldAltF);

    userSettings::setBitePoint(CLUTCH_DEFAULT_VALUE);

    Serial.println("-- GO --");
    inputs::start();
    inputs::recalibrateAxes();
}

void loop()
{
    clutchFunction_t newCP = userSettings::cpWorkingMode;
    bool newAltF = userSettings::altButtonsWorkingMode;
    if (newCP != oldCP)
    {
        oldCP = newCP;
        serialPrintf("Clutch Mode: %d\n", oldCP);
    }
    if (newAltF != oldAltF)
    {
        oldAltF = newAltF;
        serialPrintf("ALT Mode: %d\n", oldAltF);
    }
    delay(500);
}