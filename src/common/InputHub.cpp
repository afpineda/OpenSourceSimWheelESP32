/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Implementation of the `inputHub` namespace
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include "SimWheel.h"
// #include <FreeRTOS.h>

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

// Related to ALT buttons
static inputBitmap_t altBitmap = 0;

// Related to clutch calibration
#define CALIBRATION_INCREMENT 3
// #define MAX_BITEPOINT CLUTCH_FULL_VALUE-CALIBRATION_INCREMENT
// #define MIN_BITEPOINT CLUTCH_NONE_VALUE+CALIBRATION_INCREMENT
static inputBitmap_t calibrateUpBitmap = 0;
static inputBitmap_t calibrateDownBitmap = 0;

// Related to wheel functions
static inputBitmap_t cycleALTFunctionBitmap = 0;
static inputBitmap_t cycleClutchFunctionBitmap = 0;
static inputBitmap_t setClutchFunctionBitmapForCP = 0;
static inputBitmap_t setAxisFunctionBitmapForCP = 0;
static inputBitmap_t setAltFunctionBitmapForCP = 0;
static inputBitmap_t setButtonFunctionBitmapForCP = 0;

// Related to POV buttons
#define DPAD_CENTERED 0
#define DPAD_UP 1
#define DPAD_UP_RIGHT 2
#define DPAD_RIGHT 3
#define DPAD_DOWN_RIGHT 4
#define DPAD_DOWN 5
#define DPAD_DOWN_LEFT 6
#define DPAD_LEFT 7
#define DPAD_UP_LEFT 8
static inputBitmap_t dpadBitmap[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
static inputBitmap_t dpadNegMask = 0;
static inputBitmap_t dpadMask = ~0ULL;

// ----------------------------------------------------------------------------
// Input Handler
// ----------------------------------------------------------------------------

void inputHub::onStateChanged(inputBitmap_t globalState, inputBitmap_t changes)
{
    // Look for input events requesting a change in simwheel functions
    // These input events never translate into a HID report
    if ((changes & cycleALTFunctionBitmap) && (globalState == cycleALTFunctionBitmap))
    {
        clutchState::setALTModeForALTButtons(!clutchState::altModeForAltButtons);
        return;
    }
    if ((changes & cycleClutchFunctionBitmap) && (globalState == cycleClutchFunctionBitmap))
    {
        int f = clutchState::currentFunction + 1;
        if (f > CF_BUTTON)
            f = CF_CLUTCH;
        clutchState::setFunction((clutchFunction_t)f);
        return;
    }
    if ((changes & setClutchFunctionBitmapForCP) && (globalState == setClutchFunctionBitmapForCP))
    {
        clutchState::setFunction(CF_CLUTCH);
        return;
    }
    if ((changes & setAxisFunctionBitmapForCP) && (globalState == setAxisFunctionBitmapForCP))
    {
        clutchState::setFunction(CF_AXIS);
        return;
    }
    if ((changes & setAltFunctionBitmapForCP) && (globalState == setAltFunctionBitmapForCP))
    {
        clutchState::setFunction(CF_ALT);
        return;
    }
    if ((changes & setButtonFunctionBitmapForCP) && (globalState == setButtonFunctionBitmapForCP))
    {
        clutchState::setFunction(CF_BUTTON);
        return;
    }

    // Check alt mode
    inputBitmap_t filteredInputs;
    bool altEnabled = clutchState::isALTRequested();
    if (clutchState::altModeForAltButtons)
    {
        filteredInputs = altBitmap;
        altEnabled |= (globalState & altBitmap);
    }
    else
    {
        filteredInputs = 0;
    }
    
    // bite point calibration
    if (clutchState::isCalibrationInProgress())
    {
        // One and only one clutch paddle is pressed
        // Check for bite point calibration events
        int aux;
        if ((calibrateUpBitmap & changes) &&
            (calibrateUpBitmap & globalState) &&
            (clutchState::bitePoint < CLUTCH_FULL_VALUE))
        {
            aux = clutchState::bitePoint + CALIBRATION_INCREMENT;
            if (aux>CLUTCH_FULL_VALUE)
                aux = CLUTCH_FULL_VALUE;
            clutchState::setBitePoint((clutchValue_t)aux);
        }
        else if ((calibrateDownBitmap & changes) &&
                 (calibrateDownBitmap & globalState) &&
                 (clutchState::bitePoint > CLUTCH_NONE_VALUE))
        {
            aux = clutchState::bitePoint - CALIBRATION_INCREMENT;
            if (aux<CLUTCH_NONE_VALUE)
                aux = CLUTCH_NONE_VALUE;
            clutchState::setBitePoint((clutchValue_t)aux);
        }
        filteredInputs |= (calibrateDownBitmap | calibrateUpBitmap);
    }

    // Report
    globalState = globalState & (~filteredInputs);

    uint8_t povInput = DPAD_CENTERED;
    if (!altEnabled)
    {
        // Map directional pad buttons to POV input as needed
        inputBitmap_t povState = globalState & dpadNegMask;

        if (povState)
        {
            uint8_t n = 1;
            while ((povInput == 0) && (n < 9))
            {
                if (povState == dpadBitmap[n])
                    povInput = n;
                n++;
            }
        }
        globalState = globalState & (dpadMask);
    }
    hidImplementation::reportInput(globalState, altEnabled, povInput);
}

// ----------------------------------------------------------------------------
// Setup
// ----------------------------------------------------------------------------

void inputHub::setClutchCalibrationButtons(
    const inputNumber_t upButtonNumber,
    const inputNumber_t downButtonNumber)
{
    if (upButtonNumber != UNSPECIFIED_INPUT_NUMBER)
        calibrateUpBitmap = BITMAP(upButtonNumber);
    else
        calibrateUpBitmap = 0;

    if (downButtonNumber != UNSPECIFIED_INPUT_NUMBER)
        calibrateDownBitmap = BITMAP(downButtonNumber);
    else
        calibrateDownBitmap = 0;
}

void inputHub::setALTBitmap(const inputBitmap_t altBmp)
{
    altBitmap = altBmp;
    capabilities::setFlag(deviceCapability_t::CAP_ALT, (altBitmap != 0));
}

void inputHub::setALTButton(const inputNumber_t altNumber)
{
    if (altNumber == UNSPECIFIED_INPUT_NUMBER)
        altBitmap = 0;
    else
        altBitmap = BITMAP(altNumber);
    capabilities::setFlag(deviceCapability_t::CAP_ALT, (altBitmap != 0));
}

void inputHub::setDPADControls(
    inputNumber_t padUpNumber,
    inputNumber_t padDownNumber,
    inputNumber_t padLeftNumber,
    inputNumber_t padRightNumber,
    inputNumber_t padUpLeftNumber,
    inputNumber_t padUpRightNumber,
    inputNumber_t padDownLeftNumber,
    inputNumber_t padDownRightNumber)
{
    if (padUpNumber < UNSPECIFIED_INPUT_NUMBER)
        dpadBitmap[DPAD_UP] = BITMAP(padUpNumber);
    else
        dpadBitmap[DPAD_UP] = 0;

    if (padDownNumber < UNSPECIFIED_INPUT_NUMBER)
        dpadBitmap[DPAD_DOWN] = BITMAP(padDownNumber);
    else
        dpadBitmap[DPAD_DOWN] = 0;

    if (padLeftNumber < UNSPECIFIED_INPUT_NUMBER)
        dpadBitmap[DPAD_LEFT] = BITMAP(padLeftNumber);
    else
        dpadBitmap[DPAD_LEFT] = 0;

    if (padRightNumber < UNSPECIFIED_INPUT_NUMBER)
        dpadBitmap[DPAD_RIGHT] = BITMAP(padRightNumber);
    else
        dpadBitmap[DPAD_RIGHT] = 0;

    if (padUpLeftNumber < UNSPECIFIED_INPUT_NUMBER)
        dpadBitmap[DPAD_UP_LEFT] = BITMAP(padUpLeftNumber);
    else
        dpadBitmap[DPAD_UP_LEFT] = dpadBitmap[DPAD_UP] | dpadBitmap[DPAD_LEFT];

    if (padUpRightNumber < UNSPECIFIED_INPUT_NUMBER)
        dpadBitmap[DPAD_UP_RIGHT] = BITMAP(padUpRightNumber);
    else
        dpadBitmap[DPAD_UP_RIGHT] = dpadBitmap[DPAD_UP] | dpadBitmap[DPAD_RIGHT];

    if (padDownLeftNumber < UNSPECIFIED_INPUT_NUMBER)
        dpadBitmap[DPAD_DOWN_LEFT] = BITMAP(padDownLeftNumber);
    else
        dpadBitmap[DPAD_DOWN_LEFT] = dpadBitmap[DPAD_DOWN] | dpadBitmap[DPAD_LEFT];

    if (padDownRightNumber < UNSPECIFIED_INPUT_NUMBER)
        dpadBitmap[DPAD_DOWN_RIGHT] = BITMAP(padDownRightNumber);
    else
        dpadBitmap[DPAD_DOWN_RIGHT] = dpadBitmap[DPAD_DOWN] | dpadBitmap[DPAD_RIGHT];

    dpadNegMask = 0;
    for (int n = 1; n < 9; n++)
    {
        dpadNegMask |= dpadBitmap[n];
    }
    dpadMask = ~dpadNegMask;
    capabilities::setFlag(deviceCapability_t::CAP_DPAD, (dpadNegMask != 0));
}

void inputHub::setCycleALTFunctionBitmap(const inputBitmap_t bitmap)
{
    cycleALTFunctionBitmap = bitmap;
}

void inputHub::setCycleClutchFunctionBitmap(const inputBitmap_t bitmap)
{
    cycleClutchFunctionBitmap = bitmap;
}

void inputHub::setSelectClutchFunctionBitmaps(
    const inputBitmap_t clutchModeBitmap,
    const inputBitmap_t axisModeBitmap,
    const inputBitmap_t altModeBitmap,
    const inputBitmap_t buttonModeBitmap)
{
    setClutchFunctionBitmapForCP = clutchModeBitmap;
    setAxisFunctionBitmapForCP = axisModeBitmap;
    setAltFunctionBitmapForCP = altModeBitmap;
    setButtonFunctionBitmapForCP = buttonModeBitmap;
}