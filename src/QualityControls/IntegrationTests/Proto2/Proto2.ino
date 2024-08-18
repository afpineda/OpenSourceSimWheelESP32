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
    power::begin(TEST_ROTARY_SW);

   setDebugInputNumbers(
        inputs::addButtonMatrix(mtxSelectors, mtxInputs));
    inputs::addDigital(TEST_ROTARY_SW, ALT_IN );
    inputs::addRotaryEncoder(TEST_ROTARY_CLK, TEST_ROTARY_DT, CW_IN, CCW_IN,false);

    inputHub::setClutchInputNumbers(LEFT_CLUTCH_IN,RIGHT_CLUTCH_IN);
    inputHub::cycleCPWorkingMode_setInputNumbers({(COMMAND_IN),(CYCLE_CLUTCH_IN)});
    inputHub::setClutchCalibrationInputNumbers(CW_IN, CCW_IN);

    hidImplementation::begin("Proto2", "Mamandurrio", true);
    inputs::start();
    batteryMonitor::configureForTesting();
    batteryMonitor::begin(TEST_BATTERY_READ_ENABLE,TEST_BATTERY_READ);
}

void loop()
{
    vTaskDelay(portMAX_DELAY);
}