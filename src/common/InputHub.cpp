/**
 * @file InputHub.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Implementation of the `inputHub` namespace
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include "SimWheel.h"

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

// Related to ALT buttons

static inputBitmap_t altBitmap = 0;

// Related to clutch

#define CALIBRATION_INCREMENT 3
static inputBitmap_t calibrateUpBitmap = 0;
static inputBitmap_t calibrateDownBitmap = 0;
static inputBitmap_t leftClutchBitmap = 0ULL;
static inputBitmap_t rightClutchBitmap = 0ULL;
static inputBitmap_t clutchInputMask = ~0ULL;

// Related to wheel functions

static inputBitmap_t cycleALTWorkingModeBitmap = 0;
static inputBitmap_t cycleClutchWorkingModeBitmap = 0;
static inputBitmap_t cmdCPWorkingModeBitmap_clutch = 0;
static inputBitmap_t cmdCPWorkingModeBitmap_axis = 0;
static inputBitmap_t cmdCPWorkingModeBitmap_alt = 0;
static inputBitmap_t cmdCPWorkingModeBitmap_button = 0;
static inputBitmap_t cmdAxisAutocalibrationBitmap = 0;
static inputBitmap_t cmdBatteryRecalibrationBitmap = 0;

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
static inputBitmap_t dpadBitmap[9] = {0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL};
static inputBitmap_t dpadNegMask = 0ULL;
static inputBitmap_t dpadMask = ~0ULL;

// ----------------------------------------------------------------------------
// Input processing
// ----------------------------------------------------------------------------

/**
 * @brief Executes user-requested commands in response to button press combinations
 *
 * @return true If a command has been issued
 * @return false Otherwise
 */
bool inputHub_commands_filter(
    inputBitmap_t globalState,
    inputBitmap_t changes)
{
    // Look for input events requesting a change in functionality
    // These input events never translate into a HID report
    if ((changes & cycleALTWorkingModeBitmap) && (globalState == cycleALTWorkingModeBitmap))
    {
        userSettings::setALTButtonsWorkingMode(!userSettings::altButtonsWorkingMode);
        return true;
    }
    if ((changes & cycleClutchWorkingModeBitmap) && (globalState == cycleClutchWorkingModeBitmap))
    {
        int f = userSettings::cpWorkingMode + 1;
        if (f > CF_BUTTON)
            f = CF_CLUTCH;
        userSettings::setCPWorkingMode((clutchFunction_t)f);
        return true;
    }
    if ((changes & cmdCPWorkingModeBitmap_clutch) && (globalState == cmdCPWorkingModeBitmap_clutch))
    {
        userSettings::setCPWorkingMode(CF_CLUTCH);
        return true;
    }
    if ((changes & cmdCPWorkingModeBitmap_axis) && (globalState == cmdCPWorkingModeBitmap_axis))
    {
        userSettings::setCPWorkingMode(CF_AXIS);
        return true;
    }
    if ((changes & cmdCPWorkingModeBitmap_alt) && (globalState == cmdCPWorkingModeBitmap_alt))
    {
        userSettings::setCPWorkingMode(CF_ALT);
        return true;
    }
    if ((changes & cmdCPWorkingModeBitmap_button) && (globalState == cmdCPWorkingModeBitmap_button))
    {
        userSettings::setCPWorkingMode(CF_BUTTON);
        return true;
    }
    if ((changes & cmdAxisAutocalibrationBitmap) && (globalState == cmdAxisAutocalibrationBitmap))
    {
        inputs::recalibrateAxes();
        return true;
    }
    if ((changes & cmdBatteryRecalibrationBitmap) && (globalState == cmdBatteryRecalibrationBitmap))
    {
        batteryCalibration::restartAutoCalibration();
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------

// #include <HardwareSerial.h> // Debug

/**
 * @brief Executes bite point calibration from user input
 *
 */
void inputHub_bitePointCalibration_filter(
    inputBitmap_t &globalState,
    inputBitmap_t &changes,
    clutchValue_t leftAxis,
    clutchValue_t rightAxis)
{
    if (userSettings::cpWorkingMode != CF_CLUTCH)
        // not in clutch mode
        return;

    bool isCalibrationInProgress =
        ((leftAxis == CLUTCH_FULL_VALUE) && (rightAxis == CLUTCH_NONE_VALUE));
    isCalibrationInProgress =
        isCalibrationInProgress ||
        ((leftAxis == CLUTCH_NONE_VALUE) && (rightAxis == CLUTCH_FULL_VALUE));
    if (isCalibrationInProgress)
    {
        // Serial.println("isCalibrationInProgress");
        // One and only one clutch paddle is pressed
        // Check for bite point calibration events
        int aux;
        if ((calibrateUpBitmap & changes) &&
            (calibrateUpBitmap & globalState) &&
            (userSettings::bitePoint < CLUTCH_FULL_VALUE))
        {
            aux = userSettings::bitePoint + CALIBRATION_INCREMENT;
            if (aux > CLUTCH_FULL_VALUE)
                aux = CLUTCH_FULL_VALUE;
            userSettings::setBitePoint((clutchValue_t)aux);
        }
        else if ((calibrateDownBitmap & changes) &&
                 (calibrateDownBitmap & globalState) &&
                 (userSettings::bitePoint > CLUTCH_NONE_VALUE))
        {
            aux = userSettings::bitePoint - CALIBRATION_INCREMENT;
            if (aux < CLUTCH_NONE_VALUE)
                aux = CLUTCH_NONE_VALUE;
            userSettings::setBitePoint((clutchValue_t)aux);
        }
        globalState &= (~(calibrateDownBitmap | calibrateUpBitmap));
        changes &= (~(calibrateDownBitmap | calibrateUpBitmap));
    }
}

// ----------------------------------------------------------------------------

/**
 * @brief Transforms input state into axis position and vice-versa, depending on
 *        working mode of the clutch paddles.
 *
 */
void inputHub_AxisButton_filter(
    inputBitmap_t &rawInputBitmap,
    inputBitmap_t &rawInputChanges,
    clutchValue_t &leftAxis,
    clutchValue_t &rightAxis,
    bool axesAvailable)
{
    if (axesAvailable && (userSettings::cpWorkingMode == CF_BUTTON))
    {
        // Transform analog axis position into an input state
        if (leftAxis >= CLUTCH_3_4_VALUE)
        {
            rawInputBitmap |= leftClutchBitmap;
            rawInputChanges |= leftClutchBitmap;
        }
        else if (leftAxis <= CLUTCH_1_4_VALUE)
        {
            rawInputBitmap &= (~leftClutchBitmap);
            rawInputChanges |= leftClutchBitmap;
        }
        if (rightAxis >= CLUTCH_3_4_VALUE)
        {
            rawInputBitmap |= rightClutchBitmap;
            rawInputChanges |= rightClutchBitmap;
        }
        else if (rightAxis <= CLUTCH_1_4_VALUE)
        {
            rawInputBitmap &= (~rightClutchBitmap);
            rawInputChanges |= rightClutchBitmap;
        }
        leftAxis = CLUTCH_NONE_VALUE;
        rightAxis = CLUTCH_NONE_VALUE;
    }
    else if ((!axesAvailable) &&
             ((userSettings::cpWorkingMode == CF_AXIS) || (userSettings::cpWorkingMode == CF_CLUTCH)))
    {
        // Transform input state into an axis position
        if (rawInputBitmap & leftClutchBitmap)
            leftAxis = CLUTCH_FULL_VALUE;
        else
            leftAxis = CLUTCH_NONE_VALUE;
        if (rawInputBitmap & rightClutchBitmap)
            rightAxis = CLUTCH_FULL_VALUE;
        else
            rightAxis = CLUTCH_NONE_VALUE;
        rawInputChanges = (rawInputChanges & clutchInputMask);
        rawInputBitmap = (rawInputBitmap & clutchInputMask);
    }
}

// ----------------------------------------------------------------------------

/**
 * @brief Computes a combined clutch position from analog axes, when needed
 *
 */
void inputHub_combinedAxis_filter(
    clutchValue_t &leftAxis,
    clutchValue_t &rightAxis,
    clutchValue_t &clutchAxis)
{
    if (userSettings::cpWorkingMode == CF_CLUTCH)
    {
        if (leftAxis > rightAxis)
            clutchAxis =
                (leftAxis * userSettings::bitePoint +
                 (rightAxis * (255 - userSettings::bitePoint))) /
                255;
        else
            clutchAxis =
                (rightAxis * userSettings::bitePoint +
                 (leftAxis * (255 - userSettings::bitePoint))) /
                255;
        leftAxis = CLUTCH_NONE_VALUE;
        rightAxis = CLUTCH_NONE_VALUE;
    }
    else if (userSettings::cpWorkingMode == CF_AXIS)
    {
        clutchAxis = CLUTCH_NONE_VALUE;
    }
    else
    {
        leftAxis = CLUTCH_NONE_VALUE;
        rightAxis = CLUTCH_NONE_VALUE;
        clutchAxis = CLUTCH_NONE_VALUE;
    }
}

// ----------------------------------------------------------------------------

/**
 * @brief Check if ALT mode is requested by the user
 *
 */
void inputHub_AltRequest_filter(
    inputBitmap_t &rawInputBitmap,
    clutchValue_t &leftAxis,
    clutchValue_t &rightAxis,
    bool &isAltRequested)
{
    if (userSettings::altButtonsWorkingMode)
    {
        isAltRequested = (rawInputBitmap & altBitmap);
        rawInputBitmap &= ~altBitmap;
    }
    if (userSettings::cpWorkingMode == CF_ALT)
    {
        leftAxis = CLUTCH_NONE_VALUE;
        rightAxis = CLUTCH_NONE_VALUE;
        isAltRequested = isAltRequested ||
                         (leftAxis >= CLUTCH_DEFAULT_VALUE) ||
                         (rightAxis >= CLUTCH_DEFAULT_VALUE);
    }
}

// ----------------------------------------------------------------------------

void inputHub_DPAD_filter(
    inputBitmap_t &rawInputBitmap,
    uint8_t &povInput)
{
    povInput = DPAD_CENTERED;
    if (true) // TO DO: user-defined working mode of DPAD
    {
        // Map directional buttons to POV input as needed
        inputBitmap_t povState = rawInputBitmap & dpadNegMask;

        if (povState)
        {
            uint8_t n = 1;
            while ((povInput == DPAD_CENTERED) && (n < 9))
            {
                if (povState == dpadBitmap[n])
                    povInput = n;
                n++;
            }
        }
        rawInputBitmap = rawInputBitmap & dpadMask;
    }
}

// ----------------------------------------------------------------------------

void inputHub ::onRawInput(
    inputBitmap_t rawInputBitmap,
    inputBitmap_t rawInputChanges,
    clutchValue_t leftAxis,
    clutchValue_t rightAxis,
    bool axesAvailable)
{
    // Step 1: Execute user commands if any
    if (inputHub_commands_filter(rawInputBitmap, rawInputChanges))
        return;

    // Serial.printf("after step 1: %llx %u %u\n", rawInputBitmap, leftAxis, rightAxis);

    // Step 2: digital input <--> analog axes
    inputHub_AxisButton_filter(rawInputBitmap, rawInputChanges, leftAxis, rightAxis, axesAvailable);

    // Serial.printf("after step 2: %llx %u %u\n", rawInputBitmap, leftAxis, rightAxis);

    // Step 3: bite point calibration
    inputHub_bitePointCalibration_filter(rawInputBitmap, rawInputChanges, leftAxis, rightAxis);

    // Serial.printf("after step 3: %llx %u %u\n", rawInputBitmap, leftAxis, rightAxis);

    // Step 4: check if ALT mode is requested
    bool isALTRequested = false;
    inputHub_AltRequest_filter(rawInputBitmap, leftAxis, rightAxis, isALTRequested);

    // Serial.printf("after step 4: %llx %u %u\n", rawInputBitmap, leftAxis, rightAxis);

    // Step 5: compute F1-style clutch position
    clutchValue_t clutchAxis;
    inputHub_combinedAxis_filter(leftAxis, rightAxis, clutchAxis);

    // Serial.printf("after step 5: %llx %u %u\n", rawInputBitmap, leftAxis, rightAxis);

    // Step 6: compute DPAD input
    uint8_t povInput = DPAD_CENTERED;
    if (!isALTRequested)
        inputHub_DPAD_filter(rawInputBitmap, povInput);

    // Serial.printf("after step 6: %llx %u %u\n", rawInputBitmap, leftAxis, rightAxis);

    // Step 7: map raw input state into HID button state
    inputBitmap_t inputsLow, inputsHigh;
    // TO DO: apply user defined map
    if (isALTRequested)
    {
        inputsLow = 0;
        inputsHigh = rawInputBitmap;
    }
    else
    {
        inputsLow = rawInputBitmap;
        inputsHigh = 0;
    }

    // Step 8: send HID report
    hidImplementation::reportInput(inputsLow, inputsHigh, povInput, leftAxis, rightAxis, clutchAxis);
}

// ----------------------------------------------------------------------------
// Setup
// ----------------------------------------------------------------------------

void inputHub::setClutchInputNumbers(
    const inputNumber_t leftClutchInputNumber,
    const inputNumber_t rightClutchInputNumber)
{
    if ((leftClutchInputNumber <= MAX_INPUT_NUMBER) &&
        (rightClutchInputNumber <= MAX_INPUT_NUMBER) && (leftClutchInputNumber != rightClutchInputNumber))
    {
        leftClutchBitmap = BITMAP(leftClutchInputNumber);
        rightClutchBitmap = BITMAP(rightClutchInputNumber);
        clutchInputMask = ~(leftClutchBitmap | rightClutchBitmap);
        if (!capabilities::hasFlag(CAP_CLUTCH_ANALOG))
            capabilities::setFlag(CAP_CLUTCH_BUTTON);
    }
}

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

// ----------------------------------------------------------------------------

void inputHub::setALTBitmap(const inputBitmap_t altBmp)
{
    altBitmap = altBmp;
    capabilities::setFlag(deviceCapability_t::CAP_ALT, (altBitmap != 0));
}

// ----------------------------------------------------------------------------

void inputHub::setALTButton(const inputNumber_t altNumber)
{
    if (altNumber == UNSPECIFIED_INPUT_NUMBER)
        altBitmap = 0;
    else
        altBitmap = BITMAP(altNumber);
    capabilities::setFlag(deviceCapability_t::CAP_ALT, (altBitmap != 0));
}

// ----------------------------------------------------------------------------

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

    dpadNegMask = 0ULL;
    for (int n = 1; n < 9; n++)
    {
        dpadNegMask |= dpadBitmap[n];
    }
    dpadMask = ~dpadNegMask;
    capabilities::setFlag(deviceCapability_t::CAP_DPAD, (dpadNegMask != 0));
}

// ----------------------------------------------------------------------------

void inputHub::cycleALTButtonsWorkingMode_setBitmap(const inputBitmap_t bitmap)
{
    cycleALTWorkingModeBitmap = bitmap;
}

// ----------------------------------------------------------------------------

void inputHub::cycleCPWorkingMode_setBitmap(const inputBitmap_t bitmap)
{
    cycleClutchWorkingModeBitmap = bitmap;
}

// ----------------------------------------------------------------------------

void inputHub::cpWorkingMode_setBitmaps(
    const inputBitmap_t clutchModeBitmap,
    const inputBitmap_t axisModeBitmap,
    const inputBitmap_t altModeBitmap,
    const inputBitmap_t buttonModeBitmap)
{
    cmdCPWorkingModeBitmap_clutch = clutchModeBitmap;
    cmdCPWorkingModeBitmap_axis = axisModeBitmap;
    cmdCPWorkingModeBitmap_alt = altModeBitmap;
    cmdCPWorkingModeBitmap_button = buttonModeBitmap;
}

// ----------------------------------------------------------------------------

void inputHub::cmdRecalibrateAnalogAxis_setBitmap(const inputBitmap_t recalibrateAxisBitmap)
{
    cmdAxisAutocalibrationBitmap = recalibrateAxisBitmap;
}

// ----------------------------------------------------------------------------

void inputHub::cmdRecalibrateBattery_setBitmap(const inputBitmap_t recalibrateBatteryBitmap)
{

    cmdBatteryRecalibrationBitmap = recalibrateBatteryBitmap;
}

// ----------------------------------------------------------------------------
