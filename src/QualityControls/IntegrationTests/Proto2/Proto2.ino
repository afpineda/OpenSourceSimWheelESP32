/**
 * @file Proto2.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-04-21
 * @brief Integration test. See [Readme](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "debugUtils.h"
#include "SimWheel.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

#define LEFT_CLUTCH_IN 3
#define RIGHT_CLUTCH_IN 2
#define ALT_IN 10
#define CW_IN 12
#define CCW_IN 13
#define COMMAND_IN 7
//#define CYCLE_ALT_IN 5
#define CYCLE_CLUTCH_IN 6

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);

    userSettings::begin();
    power::begin(TEST_ROTARY_SW,false);

    inputs::addButtonMatrix(
        mtxSelectors,
        sizeof(mtxSelectors) / sizeof(mtxSelectors[0]),
        mtxInputs,
        sizeof(mtxInputs) / sizeof(mtxInputs[0]),
        mtxNumbers);
    inputs::addDigital(TEST_ROTARY_SW, ALT_IN, true, true );
    inputs::addRotaryEncoder(TEST_ROTARY_CLK, TEST_ROTARY_DT, CW_IN, CCW_IN,false);

    inputHub::setClutchInputNumbers(LEFT_CLUTCH_IN,RIGHT_CLUTCH_IN);
    inputHub::cycleCPWorkingMode_setBitmap(BITMAP(COMMAND_IN)|BITMAP(CYCLE_CLUTCH_IN));
    inputHub::setClutchCalibrationButtons(CW_IN, CCW_IN);

    hidImplementation::begin("Proto2", "Mamandurrio", true);
    inputs::start();
    power::startBatteryMonitor(TEST_BATTERY_READ_ENABLE,TEST_BATTERY_READ,false);
}

void loop()
{
    vTaskDelay(portMAX_DELAY);
}