/**
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

static bool inMenu = false;

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

bool configMenu::toggle()
{
    Serial.print("MENU TOGGLE: ");
    inMenu = !inMenu;
    Serial.println(inMenu);
    return inMenu;
}

void configMenu::onInput(inputBitmap_t globalState, inputBitmap_t changes)
{
    Serial.print("MENU: ");
    debugPrintBool(globalState);
    Serial.println("");
}

void hidImplementation::reportInput(inputBitmap_t globalState, bool altEnabled, clutchValue_t clutchValue, uint8_t POV)
{
    Serial.print("HID: ");
    debugPrintBool(globalState);
    Serial.print(" | ");
    Serial.print(clutchValue);
    if (altEnabled)
        Serial.print(" (ALT)");
    Serial.println("");
}

void hidImplementation::reset()
{
    Serial.println("HID RESET");
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    inputNumber_t cl1Num, cl2Num, altNum, menuNum, calUpNum, calDownNum;

    Serial.begin(115200);
    while (!Serial)
        ;

    Serial.println("-- READY --");
    inputs::begin();
    inputHub::begin();
    
    cl1Num = inputs::setButtonMatrix(mtxSelectors,3,mtxInputs,2);
    cl2Num = cl1Num+1;
    menuNum = cl2Num+1;
    altNum = inputs::addDigital(TEST_ROTARY_SW,true,false);
    calUpNum = inputs::addRotaryEncoder(TEST_ROTARY_CLK,TEST_ROTARY_DT);
    calDownNum = calUpNum+1;

    inputHub::setClutchPaddles(cl1Num,cl2Num);
    inputHub::setALTButton(altNum);
    inputHub::setClutchCalibrationButtons(calUpNum,calDownNum);
    inputHub::setMenuBitmap(BITMAP(menuNum)|BITMAP(menuNum+1));

    inputHub::setClutchFunction(CF_CLUTCH);
    inputHub::setALTFunction(true);
    inputHub::setClutchBitePoint(0);

    Serial.println("-- GO --");
    inputs::start();
}

void loop()
{
    delay(5000);
}