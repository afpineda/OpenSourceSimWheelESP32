/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Dummy implementation of the `inputHub` namespace
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.h"

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

clutchValue_t mockBitePoint = CLUTCH_DEFAULT_VALUE;
clutchFunction_t mockClutchFuncion = CF_CLUTCH;
bool mockAltFunction = true;
volatile clutchValue_t leftClutchValue;
volatile clutchValue_t rightClutchValue;

// ----------------------------------------------------------------------------
// Input Handler
// ----------------------------------------------------------------------------

void inputHub::onStateChanged(inputBitmap_t globalState, inputBitmap_t changes)
{

}

// ----------------------------------------------------------------------------
// Setup
// ----------------------------------------------------------------------------

void inputHub::setClutchPaddles(
    const inputNumber_t leftClutchNumber,
    const inputNumber_t rightClutchNumber)
{

}

void inputHub::setClutchCalibrationButtons(
    const inputNumber_t upButtonNumber,
    const inputNumber_t downButtonNumber)
{

}

void inputHub::setMenuButton(const inputNumber_t menuButtonNumber)
{

}

void inputHub::setALTBitmap(const inputBitmap_t altBmp)
{

}

void inputHub::setALTButton(const inputNumber_t altNumber)
{

}

// ----------------------------------------------------------------------------
// Setters
// ----------------------------------------------------------------------------

void inputHub::setClutchBitePoint(clutchValue_t calibrationValue, bool save)
{
    mockBitePoint = calibrationValue;
}

void inputHub::setClutchFunction(clutchFunction_t newFunction, bool save)
{
   mockClutchFuncion = newFunction;
}

void inputHub::setALTFunction(bool altFunction, bool save)
{
    mockAltFunction = altFunction;
}

void inputHub::notifyMenuExit()
{

}

// ----------------------------------------------------------------------------
// Getters
// ----------------------------------------------------------------------------

clutchValue_t inputHub::getClutchBitePoint()
{
    return mockBitePoint;
}

clutchFunction_t inputHub::getClutchFunction()
{
    return mockClutchFuncion;
}

bool inputHub::getALTFunction()
{
    return mockAltFunction;
}

bool inputHub::hasClutchPaddles()
{
    return true;
}

bool inputHub::hasALTButtons()
{
    return true;
}

void inputHub::begin()
{

}