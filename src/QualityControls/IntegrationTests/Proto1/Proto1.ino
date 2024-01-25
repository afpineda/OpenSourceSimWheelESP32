/**
 * @file Proto1.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-01
 * @brief Integration test. See [Readme](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <HardwareSerial.h>
#include "debugUtils.h"
#include "SimWheel.h"
#include "SerialNotification.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

#define LEFT_CLUTCH_IN 0
#define RIGHT_CLUTCH_IN 1
#define ALT_IN 10
#define CW_IN 12
#define CCW_IN 13
#define COMMAND_IN 2
#define CYCLE_ALT_IN 3
#define CYCLE_CLUTCH_IN 4
#define CYCLE_DPAD_IN 5
#define UP 3
#define DOWN 4
#define LEFT 5
#define RIGHT 6

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

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

    notify::begin(new SerialNotificationImpl());

    inputs::addButtonMatrix(
        mtxSelectors,
        sizeof(mtxSelectors) / sizeof(mtxSelectors[0]),
        mtxInputs,
        sizeof(mtxInputs) / sizeof(mtxInputs[0]),
        mtxNumbers);
    inputs::addDigital(TEST_ROTARY_SW, ALT_IN, true, true);
    inputs::addRotaryEncoder(TEST_ROTARY_CLK, TEST_ROTARY_DT, CW_IN, CCW_IN, false);
    inputs::setAnalogClutchPaddles(TEST_ANALOG_PIN1, TEST_ANALOG_PIN2);

    inputHub::setALTButton(ALT_IN);
    inputHub::cycleALTButtonsWorkingMode_setBitmap(BITMAP(COMMAND_IN) | BITMAP(CYCLE_ALT_IN));
    inputHub::cycleCPWorkingMode_setBitmap(BITMAP(COMMAND_IN) | BITMAP(CYCLE_CLUTCH_IN));
    inputHub::cycleDPADWorkingMode_setBitmap(BITMAP(COMMAND_IN) | BITMAP(CYCLE_DPAD_IN));
    inputHub::setClutchInputNumbers(LEFT_CLUTCH_IN, RIGHT_CLUTCH_IN);
    inputHub::setClutchCalibrationButtons(CW_IN, CCW_IN);
    inputHub::setDPADControls(UP, DOWN, LEFT, RIGHT);

    userSettings::cpWorkingMode = CF_CLUTCH;
    userSettings::altButtonsWorkingMode = true;
    userSettings::dpadWorkingMode = true;
    userSettings::bitePoint = CLUTCH_DEFAULT_VALUE;

    hidImplementation::begin("Proto1", "Mamandurrio", false);

    Serial.println("-- GO --");
    inputs::start();
}

void loop()
{
    vTaskDelay(portMAX_DELAY);
}