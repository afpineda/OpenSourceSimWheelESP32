/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-04-21
 * @brief Integration test. See [Readme](./README.md)
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "debugUtils.h"
#include "SimWheel.h"

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    inputNumber_t cl1Num, cl2Num, altNum, menuNum, calUpNum, calDownNum;

    power::begin(TEST_ROTARY_SW,false);
    language::begin();
    uiManager::begin();
    ui::begin();
    batteryCalibration::begin();
    power::startBatteryMonitor(TEST_BATTERY_READ_ENABLE,TEST_BATTERY_READ);
    inputs::begin();
    inputHub::begin();

    cl1Num = inputs::setButtonMatrix(mtxSelectors, 3, mtxInputs, 2);
    cl2Num = cl1Num + 1;
    menuNum = cl2Num + 1;
    altNum = inputs::addDigital(TEST_ROTARY_SW, true, false);
    calUpNum = inputs::addRotaryEncoder(TEST_ROTARY_CLK, TEST_ROTARY_DT);
    calDownNum = calUpNum + 1;

    inputHub::setClutchPaddles(cl1Num, cl2Num);
    inputHub::setALTButton(altNum);
    inputHub::setClutchCalibrationButtons(calUpNum, calDownNum);
    inputHub::setMenuButton(menuNum);

    configMenu::setNavButtons(calDownNum, calUpNum, altNum, menuNum);

    hidImplementation::begin("Proto2", "Mamandurrio", true, true);
    inputs::start();
}

void loop()
{
    vTaskDelay(portMAX_DELAY);
}