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
#define ALT_IN 6
#define LEFT_CLUTCH_IN 0
#define RIGHT_CLUTCH_IN 1
#define CW_IN 7
#define CCW_IN 8
#define COMMAND_IN 3
#define CYCLE_CLUTCH_BMP BITMAP()
#define CYCLE_ALT_BMP 6

clutchFunction_t oldCP;
bool oldAltF;

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

void hidImplementation::reportInput(inputBitmap_t globalState, bool altEnabled, uint8_t POV)
{
    debugPrintBool(globalState);
    serialPrintf(" | %d | %d | %d ",clutchState::combinedAxis, clutchState::leftAxis, clutchState::rightAxis);
    if (altEnabled)
        Serial.print(" (ALT)");
    Serial.println("");
}

void hidImplementation::reset()
{
    Serial.println("HID RESET");
}

void hidImplementation::reportChangeInConfig()
{

}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("-- READY --");
    inputs::begin();

    inputs::addButtonMatrix(mtxSelectors, 3, mtxInputs, 2, mtxNumbers);
    inputs::addDigital(TEST_ROTARY_SW, true, true, ALT_IN);
    inputs::addRotaryEncoder(TEST_ROTARY_CLK, TEST_ROTARY_DT, CW_IN, CCW_IN);

    inputs::setDigitalClutchPaddles(LEFT_CLUTCH_IN, RIGHT_CLUTCH_IN);
    inputHub::setALTButton(ALT_IN);

    inputHub::setClutchCalibrationButtons(CW_IN, CCW_IN);

    oldCP = CF_CLUTCH;
    clutchState::setFunction(oldCP);
    oldAltF = true;
    clutchState::setALTModeForALTButtons(oldAltF);

    clutchState::setBitePoint(CLUTCH_DEFAULT_VALUE);

    Serial.println("-- GO --");
    inputs::start();
}

void loop()
{
    clutchFunction_t newCP = clutchState::currentFunction;
    bool newAltF = clutchState::altModeForAltButtons;
    if (newCP!=oldCP) {
        oldCP = newCP;
        serialPrintf("Clutch Mode: %d",oldCP);
    }
    if (newAltF!=oldAltF) {
        oldAltF = newAltF;
        serialPrintf("ALT Mode: %d",oldAltF);
    }
    delay(500);
}