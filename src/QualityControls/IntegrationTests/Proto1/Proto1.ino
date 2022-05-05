/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-01
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

void batteryCalibration::restartAutoCalibration()
{
    
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    inputNumber_t cl1Num, cl2Num, altNum, menuNum, calUpNum, calDownNum;

    language::begin();
    language::setLanguage(LANG_EN);
    uiManager::begin();
    ui::begin();
    hidImplementation::begin("Proto1", "Mamandurrio", false);
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

    inputHub::setClutchFunction(CF_CLUTCH);
    inputHub::setALTFunction(true);
    inputHub::setClutchBitePoint(0);

    configMenu::setNavButtons(calDownNum, calUpNum, altNum, menuNum);

    inputs::start();
}

void loop()
{
    delay(5000);
}