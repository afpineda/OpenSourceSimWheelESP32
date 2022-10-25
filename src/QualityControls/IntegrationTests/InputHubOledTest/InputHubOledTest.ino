/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-05
 * @brief Integration Test. See [README](./README.md)
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

static bool inMenu = false;

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

volatile char uartServer::gear;
volatile uint8_t uartServer::rpmPercent;
volatile uint16_t uartServer::speed;
volatile uint8_t uartServer::engineMap;
volatile uint8_t uartServer::absLevel;
volatile uint8_t uartServer::tcLevel;
volatile uint64_t uartServer::frameCount;

void hidImplementation::reportInput(inputBitmap_t globalState, bool altEnabled, clutchValue_t clutchValue, uint8_t POV)
{
}

void hidImplementation::reset()
{
}

int power::getLastBatteryLevel()
{
    return 01;
}

void power::powerOff(bool force)
{
}

bool power::hasBatteryMonitor()
{
    return false;
}

void batteryCalibration::restartAutoCalibration()
{
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    inputNumber_t cl1Num, cl2Num, altNum, menuNum, calUpNum, calDownNum;

    Serial.begin(115200);
    // while (!Serial)
    //     ;

    Serial.println("-- READY --");
    language::begin();
    language::setLanguage(LANG_EN);
    uiManager::begin();
    ui::begin();
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

    Serial.println("-- GO --");
    inputs::start();
}

void loop()
{
    delay(5000);
}