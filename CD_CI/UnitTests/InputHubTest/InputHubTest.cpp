/**
 * @file InputHubTest.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-10
 * @brief Unit test
 *
 * @copyright Licensed under the EUPL
 *
 */

//------------------------------------------------------------------
// Imports
//------------------------------------------------------------------

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include "cd_ci_assertions.hpp"
#include <iostream>
#include <functional>

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

#define BITMAP(N) (1ULL << N)

#define LCLUTCH 1
#define RCLUTCH 2
#define ALT_IN 3
#define CMD 4
#define CYCLE_CLUTCH 5
#define CYCLE_ALT 6
#define UP 7
#define DOWN 8
#define LEFT 9
#define RIGHT 10
#define CYCLE_DPAD 11
#define OTHER 20
#define OTHER2 21
#define OTHER3 22
#define LSHIFT 23
#define RSHIFT 24
#define NEUTRAL 25

#define OTHER_MAP 126
#define OTHER_MAP_ALT 30

#define COMBINATION_CYCLE_CLUTCH {CMD, CYCLE_CLUTCH}
#define COMBINATION_CYCLE_ALT {CMD, CYCLE_ALT}
#define COMBINATION_CYCLE_DPAD {CMD, CYCLE_DPAD}
#define COMBINATION_SECURITY_LOCK {(CMD), (OTHER), (UP)}

#define BMP_CYCLE_CLUTCH (BITMAP(CMD) | BITMAP(CYCLE_CLUTCH))
#define BMP_CYCLE_ALT (BITMAP(CMD) | BITMAP(CYCLE_ALT))
#define BMP_CYCLE_DPAD (BITMAP(CMD) | BITMAP(CYCLE_DPAD))

#define BMP_OTHER_MAP_LOW 0ULL
#define BMP_OTHER_MAP_HIGH BITMAP(OTHER_MAP - 64)
#define BMP_OTHER_MAP_ALT_LOW BITMAP(OTHER_MAP_ALT)
#define BMP_OTHER_MAP_ALT_HIGH 0ULL

#define ALT_B BITMAP(ALT_IN)
#define UP_B BITMAP(UP)
#define DOWN_B BITMAP(DOWN)
#define LEFT_B BITMAP(LEFT)
#define RIGHT_B BITMAP(RIGHT)

//------------------------------------------------------------------
// MOCKS (static)
//------------------------------------------------------------------

uint8_t currentPOV = 0;
bool currentALTEnabled = false;
uint64_t currentState = 0ULL;
uint64_t currentLow = 0ULL;
uint64_t currentHigh = 0ULL;
uint8_t currentClutch = CLUTCH_NONE_VALUE;
uint8_t currentLeftAxis = CLUTCH_NONE_VALUE;
uint8_t currentRightAxis = CLUTCH_NONE_VALUE;

void internals::hid::reset()
{
    currentPOV = 0;
    currentALTEnabled = false;
    currentState = 0;
}

void internals::hid::reportInput(
    uint64_t inputsLow,
    uint64_t inputsHigh,
    uint8_t POVstate,
    uint8_t leftAxis,
    uint8_t rightAxis,
    uint8_t clutchAxis)
{
    currentLow = inputsLow;
    currentHigh = inputsHigh;
    currentPOV = POVstate;
    currentClutch = clutchAxis;
    currentLeftAxis = leftAxis;
    currentRightAxis = rightAxis;
    currentALTEnabled = (inputsLow == 0ULL) && (inputsHigh != 0ULL);
    currentState = currentALTEnabled ? inputsHigh : inputsLow;
}

//------------------------------------------------------------------

void internals::inputMap::map(
    bool isAltModeEngaged,
    uint64_t firmware_bitmap,
    uint64_t &low,
    uint64_t &high)
{
    if (isAltModeEngaged)
    {
        low = 0ULL;
        high = firmware_bitmap;
    }
    else
    {
        low = firmware_bitmap;
        high = 0ULL;
    }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
// Auxiliary: input simulator
//------------------------------------------------------------------
//------------------------------------------------------------------

class InputSimulator
{
public:
    void push(uint8_t btnNumber);
    void pushSeveral(uint64_t bmp);
    void release(uint8_t btnNumber = 0xFF);
    void send();
    void repeat();
    void axis(uint8_t left, uint8_t right);

    InputSimulator()
    {
        event.rawInputBitmap = 0ULL;
        event.rawInputChanges = 0ULL;
        event.leftAxisValue = CLUTCH_NONE_VALUE;
        event.rightAxisValue = CLUTCH_NONE_VALUE;
    };

    DecouplingEvent event;
} input{};

void InputSimulator::push(uint8_t btnNumber)
{
    pushSeveral(BITMAP(btnNumber));
}

void InputSimulator::pushSeveral(uint64_t bmp)
{
    auto oldBitmap = event.rawInputBitmap;
    event.rawInputBitmap |= bmp;
    event.rawInputChanges = event.rawInputBitmap ^ oldBitmap;
    send();
}

void InputSimulator::release(uint8_t btnNumber)
{
    auto oldBitmap = event.rawInputBitmap;
    if (btnNumber >= 64)
        event.rawInputBitmap = 0ULL;
    else
        event.rawInputBitmap &= ~BITMAP(btnNumber);
    event.rawInputChanges = event.rawInputBitmap ^ oldBitmap;
    send();
}

void InputSimulator::send()
{
    DecouplingEvent copy = event;
    internals::inputHub::onRawInput(copy);
}

void InputSimulator::repeat()
{
    event.rawInputChanges = 0ULL;
    send();
}

void InputSimulator::axis(uint8_t left, uint8_t right)
{
    event.leftAxisValue = left;
    event.rightAxisValue = right;
    send();
}

//------------------------------------------------------------------

void noClutchPaddles()
{
    DeviceCapabilities::setFlag(DeviceCapability::CLUTCH_BUTTON, false);
    DeviceCapabilities::setFlag(DeviceCapability::CLUTCH_ANALOG, false);
}

void clutchPaddleType(bool analog)
{
    DeviceCapabilities::setFlag(DeviceCapability::CLUTCH_ANALOG, analog);
    DeviceCapabilities::setFlag(DeviceCapability::CLUTCH_BUTTON, !analog);
}

//------------------------------------------------------------------

#define pushAssertEqualsRelease(T, bmp, text, expected, found) \
    {                                                          \
        input.pushSeveral(bmp);                                \
        assert<T>::equals(text, expected, found);              \
        input.release();                                       \
    }

//------------------------------------------------------------------
//------------------------------------------------------------------
// Test groups
//------------------------------------------------------------------
//------------------------------------------------------------------

void TG_altEngagement()
{
    // Send input for ALT engagement: clutch paddles in ALT mode and ALT buttons in ALT mode,
    // test that ALT is engaged
    input.release();
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::ALT);
    InputHubService::call::setAltButtonsWorkingMode(AltButtonsWorkingMode::ALT);
    clutchPaddleType(true);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);

    // With ALT button
    input.push(ALT_IN);
    assert<uint64_t>::equals("ALT, inputs state low", 0ULL, currentLow);
    assert<uint64_t>::equals("ALT, inputs state high", 0ULL, currentHigh);
    input.push(OTHER);
    assert<uint64_t>::equals("ALT+OTHER, inputs state low", 0ULL, currentLow);
    assert<uint64_t>::equals("ALT+OTHER, inputs state high", BITMAP(OTHER), currentHigh);
    input.release();
    assert<uint64_t>::equals("ALT release, inputs state low", 0ULL, currentLow);
    assert<uint64_t>::equals("ALT release, inputs state high", 0ULL, currentHigh);

    // With analog clutch paddles
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_NONE_VALUE);
    assert<uint64_t>::equals("Analog paddle, inputs state low", 0ULL, currentLow);
    assert<uint64_t>::equals("Analog paddle, inputs state high", 0ULL, currentHigh);
    assert<uint8_t>::equals("Analog paddle, left axis", CLUTCH_NONE_VALUE, currentLeftAxis);
    input.push(OTHER);

    assert<uint64_t>::equals("Analog paddle+OTHER, inputs state low", 0ULL, currentLow);
    assert<uint64_t>::equals("Analog paddle+OTHER, inputs state high", BITMAP(OTHER), currentHigh);
    input.release();
    assert<uint64_t>::equals("Analog release, inputs state low", 0ULL, currentLow);
    assert<uint64_t>::equals("Analog release, inputs state high", 0ULL, currentHigh);

    // With digital clutch paddles
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    clutchPaddleType(false);
    input.push(LCLUTCH);
    assert<uint64_t>::equals("Digital paddle, inputs state low", 0ULL, currentLow);
    assert<uint64_t>::equals("Digital paddle, inputs state high", 0ULL, currentHigh);
    input.push(OTHER);
    assert<uint64_t>::equals("Digital paddle+OTHER, inputs state low", 0ULL, currentLow);
    assert<uint64_t>::equals("Digital paddle+OTHER, inputs state high", BITMAP(OTHER), currentHigh);
    input.release();
    assert<uint64_t>::equals("Digital release, inputs state low", 0ULL, currentLow);
    assert<uint64_t>::equals("Digital release, inputs state high", 0ULL, currentHigh);
}

void TG_altInButtonsMode()
{
    // Simulate ALT button usage while in "regular buttons" mode
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::CLUTCH);
    input.release();
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    InputHubService::call::setAltButtonsWorkingMode(AltButtonsWorkingMode::Regular);
    input.push(ALT_IN);
    assert<uint64_t>::equals("ALT push, buttons mode, low", ALT_B, currentLow);
    assert<uint64_t>::equals("ALT push, buttons mode, high", 0ULL, currentHigh);
    input.push(OTHER);
    assert<uint64_t>::equals("ALT+OTHER, buttons mode, low", ALT_B | BITMAP(OTHER), currentLow);
    assert<uint64_t>::equals("ALT+OTHER, buttons mode, high", 0ULL, currentHigh);
    input.release();
    assert<uint64_t>::equals("release, buttons mode, low", 0ULL, currentLow);
    assert<uint64_t>::equals("release, buttons mode, high", 0ULL, currentHigh);
}

void TG_POV_validInput()
{
    // Simulate a single push of each DPAD direction,
    // test POV is detected,
    // test their input numbers are NOT detected.
    InputHubService::call::setDPadWorkingMode(DPadWorkingMode::Navigation);
    input.release();
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    pushAssertEqualsRelease(uint8_t, UP_B, "UP_B", 1, currentPOV);
    pushAssertEqualsRelease(uint8_t, UP_B, "UP_B", 1, currentPOV);
    pushAssertEqualsRelease(uint8_t, DOWN_B, "DOWN_B", 5, currentPOV);
    pushAssertEqualsRelease(uint8_t, LEFT_B, "LEFT_B", 7, currentPOV);
    pushAssertEqualsRelease(uint8_t, RIGHT_B, "RIGHT_B", 3, currentPOV);
    pushAssertEqualsRelease(uint8_t, UP_B | LEFT_B, "UP_B | LEFT_B", 8, currentPOV);
    pushAssertEqualsRelease(uint8_t, DOWN_B | LEFT_B, "DOWN_B | LEFT_B", 6, currentPOV);
    pushAssertEqualsRelease(uint8_t, UP_B | RIGHT_B, "UP_B | RIGHT_B", 2, currentPOV);
    pushAssertEqualsRelease(uint8_t, DOWN_B | RIGHT_B, "DOWN_B | RIGHT_B", 4, currentPOV);
    input.push(UP);
    assert<uint64_t>::equals("state at push", 0ULL, currentState);
    input.release(UP);
    assert<uint64_t>::equals("state at release", 0ULL, currentState);
}

void TG_POV_invalidInput()
{
    // Simulate impossible DPAD input,
    // test POV is NOT detected,
    // test their input numbers are NOT detected.
    InputHubService::call::setDPadWorkingMode(DPadWorkingMode::Navigation);
    input.release();
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    pushAssertEqualsRelease(uint8_t, UP_B | DOWN_B, "UP_B | DOWN_B", 0, currentPOV);
    pushAssertEqualsRelease(uint8_t, LEFT_B | RIGHT_B, "LEFT_B | RIGHT_B", 0, currentPOV);
    input.pushSeveral(UP_B | DOWN_B);
    assert<uint64_t>::equals("state at push", 0, currentState);
    input.release();
    assert<uint64_t>::equals("state at release", 0, currentState);
}

void TG_POV_whileAlt()
{
    // Simulate DPAD operation while ALT is engaged,
    // test ALT button is not detected,
    // test POV is NOT detected,
    // test DPAD input numbers ARE detected
    InputHubService::call::setDPadWorkingMode(DPadWorkingMode::Navigation);
    InputHubService::call::setAltButtonsWorkingMode(AltButtonsWorkingMode::ALT);
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::CLUTCH);
    input.release();
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    input.push(ALT_IN);
    assert<uint64_t>::equals("ALT filtered state", 0, currentState);
    input.push(UP);
    assert<uint8_t>::equals("UP_B | ALT_B POV", 0, currentPOV);
    assert<uint64_t>::equals("UP_B | ALT_B state at push", UP_B, currentState);
    assert<bool>::equals("UP_B | ALT_B altEnabled", true, currentALTEnabled);
    input.release(UP);
    assert<uint64_t>::equals("UP_B | ALT_B state at release", 0, currentState);
    pushAssertEqualsRelease(uint8_t, DOWN_B | ALT_B, "DOWN_B | ALT_B", 0, currentPOV);
    pushAssertEqualsRelease(uint8_t, LEFT_B | ALT_B, "LEFT_B | ALT_B", 0, currentPOV);
    pushAssertEqualsRelease(uint8_t, RIGHT_B | ALT_B, "RIGHT_B | ALT_B", 0, currentPOV);
    input.release();
}

void TG_POV_ButtonsMode()
{
    // Simulate DPAD operation ,
    // test DPAD input numbers ARE detected,
    // test POV is not detected
    InputHubService::call::setDPadWorkingMode(DPadWorkingMode::Regular);
    input.release();
    input.push(UP);
    assert<uint64_t>::equals("UP, bitmap", UP_B, currentLow);
    assert<uint8_t>::equals("UP, pov", 0, currentPOV);
    input.push(RIGHT);
    assert<uint64_t>::equals("UP+RIGHT, bitmap", UP_B | RIGHT_B, currentLow);
    assert<uint8_t>::equals("UP+RIGHT, pov", 0, currentPOV);
    input.release();
    assert<uint64_t>::equals("release, bitmap", 0ULL, currentLow);
    assert<uint8_t>::equals("release, pov", 0, currentPOV);
    InputHubService::call::setDPadWorkingMode(DPadWorkingMode::Navigation);
}

void TG_cycleAlt()
{
    // Cycle working mode of ALT buttons
    InputHubService::call::setAltButtonsWorkingMode(AltButtonsWorkingMode::ALT);
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::CLUTCH);
    pushAssertEqualsRelease(int,
                            BMP_CYCLE_ALT,
                            "Cycle alt 1",
                            (int)AltButtonsWorkingMode::Regular,
                            (int)InputHubService::call::getAltButtonsWorkingMode());
    pushAssertEqualsRelease(int,
                            BMP_CYCLE_ALT, "Cycle alt 2",
                            (int)AltButtonsWorkingMode::ALT,
                            (int)InputHubService::call::getAltButtonsWorkingMode());
}

void TG_cycleClutchWorkingMode()
{
    // Cycle working mode of clutch paddles
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::LAUNCH_CONTROL_MASTER_RIGHT);
    pushAssertEqualsRelease(
        int,
        BMP_CYCLE_CLUTCH,
        "Cycle clutch 1",
        (int)ClutchWorkingMode::CLUTCH,
        (int)InputHubService::call::getClutchWorkingMode());
    pushAssertEqualsRelease(
        int,
        BMP_CYCLE_CLUTCH,
        "Cycle clutch 2",
        (int)ClutchWorkingMode::AXIS,
        (int)InputHubService::call::getClutchWorkingMode());
    pushAssertEqualsRelease(
        int,
        BMP_CYCLE_CLUTCH,
        "Cycle clutch 3",
        (int)ClutchWorkingMode::ALT,
        (int)InputHubService::call::getClutchWorkingMode());
    pushAssertEqualsRelease(
        int,
        BMP_CYCLE_CLUTCH,
        "Cycle clutch 4",
        (int)ClutchWorkingMode::BUTTON,
        (int)InputHubService::call::getClutchWorkingMode());
    pushAssertEqualsRelease(
        int,
        BMP_CYCLE_CLUTCH,
        "Cycle clutch 5",
        (int)ClutchWorkingMode::LAUNCH_CONTROL_MASTER_LEFT,
        (int)InputHubService::call::getClutchWorkingMode());
    pushAssertEqualsRelease(
        int,
        BMP_CYCLE_CLUTCH,
        "Cycle clutch 6",
        (int)ClutchWorkingMode::LAUNCH_CONTROL_MASTER_RIGHT,
        (int)InputHubService::call::getClutchWorkingMode());
}

void TG_cycleDPADWorkingMode()
{
    // Cycle working mode of DPAD inputs
    InputHubService::call::setDPadWorkingMode(DPadWorkingMode::Navigation);
    pushAssertEqualsRelease(
        int,
        BMP_CYCLE_DPAD,
        "Cycle dpad 1",
        (int)DPadWorkingMode::Regular,
        (int)InputHubService::call::getDPadWorkingMode());
    pushAssertEqualsRelease(
        int,
        BMP_CYCLE_DPAD,
        "Cycle dpad 2",
        (int)DPadWorkingMode::Navigation,
        (int)InputHubService::call::getDPadWorkingMode());
    input.release();
    InputHubService::call::setDPadWorkingMode(DPadWorkingMode::Navigation);
}

void TG_nonMappedCombinations()
{
    // Test that a button combination not mapped to a wheel command does
    // not trigger other wheel commands
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::BUTTON);
    InputHubService::call::setAltButtonsWorkingMode(AltButtonsWorkingMode::ALT);
    input.pushSeveral(BMP_CYCLE_ALT | BMP_CYCLE_CLUTCH);
    assert<int>::equals(
        "CF_BUTTON",
        (int)ClutchWorkingMode::BUTTON,
        (int)InputHubService::call::getClutchWorkingMode());
    assert<int>::equals(
        "alt mode",
        (int)AltButtonsWorkingMode::ALT,
        (int)InputHubService::call::getAltButtonsWorkingMode());
    input.release();
}

void TG_bitePointCalibration()
{
    // Simulate inputs for bite point calibration,
    // test that bite point changes as expected
    uint8_t biteP;
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::CLUTCH);
    InputHubService::call::setBitePoint(CLUTCH_DEFAULT_VALUE);
    clutchPaddleType(true);
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_NONE_VALUE);
    biteP = InputHubService::call::getBitePoint();
    input.push(UP);
    input.release(UP);
    assert((InputHubService::call::getBitePoint() > biteP) && "Invalid bite point. Expected higher.");
    biteP = InputHubService::call::getBitePoint();
    input.push(DOWN);
    input.release(DOWN);
    input.push(DOWN);
    input.release(DOWN);
    assert((InputHubService::call::getBitePoint() < biteP) && "Invalid bite point. Expected lower.");

    /// Test that the bite point calibration is not triggered without paddle operation
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    biteP = InputHubService::call::getBitePoint();
    input.push(UP);
    input.release(UP);
    assert((InputHubService::call::getBitePoint() == biteP) && "Invalid bite point. Expected no change since no clutch paddle is pressed");

    /// Test that the bite point calibration is not triggered when both paddles are pressed
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_FULL_VALUE);
    biteP = InputHubService::call::getBitePoint();
    input.push(UP);
    input.release(UP);
    assert((InputHubService::call::getBitePoint() == biteP) && "Invalid bite point. Expected no change since both clutch paddles are pressed");
}

void TG_bitePointCalibrationInLaunchControl()
{
    // Simulate inputs for bite point calibration,
    // test that bite point changes as expected

    /// --- Right paddle is master
    uint8_t biteP;
    clutchPaddleType(true);
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::LAUNCH_CONTROL_MASTER_RIGHT);
    InputHubService::call::setBitePoint(CLUTCH_DEFAULT_VALUE);
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_NONE_VALUE);
    biteP = InputHubService::call::getBitePoint();
    input.push(UP);
    input.release(UP);
    assert((InputHubService::call::getBitePoint() > biteP) &&
           "Master=right. Invalid bite point. Expected higher");
    biteP = InputHubService::call::getBitePoint();
    input.push(DOWN);
    input.release(DOWN);
    input.push(DOWN);
    input.release(DOWN);
    assert((InputHubService::call::getBitePoint() < biteP) &&
           "Master=right. Invalid bite point. Expected lower");

    //// Test that the master paddle does not trigger bite point calibration
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_FULL_VALUE);
    biteP = InputHubService::call::getBitePoint();
    input.push(UP);
    input.release(UP);
    assert((InputHubService::call::getBitePoint() == biteP) &&
           "Master=right. Invalid bite point. Expected no change");

    /// --- Left paddle is master
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::LAUNCH_CONTROL_MASTER_LEFT);
    InputHubService::call::setBitePoint(CLUTCH_DEFAULT_VALUE);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_FULL_VALUE);
    biteP = InputHubService::call::getBitePoint();
    input.push(UP);
    input.release(UP);
    assert((InputHubService::call::getBitePoint() > biteP) &&
           "Master=left. Invalid bite point. Expected higher");
    biteP = InputHubService::call::getBitePoint();
    input.push(DOWN);
    input.release(DOWN);
    input.push(DOWN);
    input.release(DOWN);
    assert((InputHubService::call::getBitePoint() < biteP) &&
           "Master=left. Invalid bite point. Expected lower");
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_FULL_VALUE);

    //// Test that the master paddle does not trigger bite point calibration
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_NONE_VALUE);
    biteP = InputHubService::call::getBitePoint();
    input.push(UP);
    input.release(UP);
    assert((InputHubService::call::getBitePoint() == biteP) &&
           "Master=left. Invalid bite point. Expected no change");

    /// Test that the bite point calibration is not triggered without paddle operation
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    biteP = InputHubService::call::getBitePoint();
    input.push(UP);
    input.release(UP);
    assert((InputHubService::call::getBitePoint() == biteP) &&
           "Master=left. Invalid bite point. Expected no change since no clutch paddle is pressed");

    /// Test that the bite point calibration is not triggered when both paddles are pressed
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_FULL_VALUE);
    biteP = InputHubService::call::getBitePoint();
    input.push(UP);
    input.release(UP);
    assert((InputHubService::call::getBitePoint() == biteP) &&
           "Master=left. Invalid bite point. Expected no change since both clutch paddles are pressed");
}

void TG_dualClutch()
{
    // Simulate clutch paddle operation,
    // Test computed clutch position
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::CLUTCH);
    InputHubService::call::setBitePoint(CLUTCH_DEFAULT_VALUE);
    clutchPaddleType(true);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    assert<uint8_t>::equals("dual clutch: 0,0", currentClutch, (uint8_t)CLUTCH_NONE_VALUE);
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_FULL_VALUE);
    assert<uint8_t>::equals("dual clutch: FULL,FULL", currentClutch, (uint8_t)CLUTCH_FULL_VALUE);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_FULL_VALUE);
    assert<uint8_t>::almostEquals("dual clutch: 0,FULL", currentClutch, (uint8_t)InputHubService::call::getBitePoint(), 1);
    InputHubService::call::setBitePoint(CLUTCH_3_4_VALUE);
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_NONE_VALUE);
    assert<uint8_t>::almostEquals("dual clutch: FULL,0", currentClutch, (uint8_t)InputHubService::call::getBitePoint(), 1);
}

void TG_launchControl()
{
    // Simulate clutch paddle operation,
    // Test computed clutch position in launch control mode
    clutchPaddleType(true);
    InputHubService::call::setBitePoint(CLUTCH_DEFAULT_VALUE);

    /// --- Left paddle is master
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::LAUNCH_CONTROL_MASTER_LEFT);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    assert<uint8_t>::equals("Launch control master left: 0,0", currentClutch, (uint8_t)CLUTCH_NONE_VALUE);
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_FULL_VALUE);
    assert<uint8_t>::equals("Launch control master left: FULL,FULL", currentClutch, (uint8_t)CLUTCH_FULL_VALUE);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_FULL_VALUE);
    assert<uint8_t>::equals("Launch control master left: 0,FULL", currentClutch, (uint8_t)CLUTCH_DEFAULT_VALUE);
    input.axis(CLUTCH_3_4_VALUE, CLUTCH_FULL_VALUE);
    assert<uint8_t>::equals("Launch control master left: 3/4,FULL", currentClutch, (uint8_t)CLUTCH_3_4_VALUE);
    input.axis(CLUTCH_3_4_VALUE, CLUTCH_NONE_VALUE);
    assert<uint8_t>::equals("Launch control master left: 3/4,0", currentClutch, (uint8_t)CLUTCH_3_4_VALUE);

    /// --- Right paddle is master
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::LAUNCH_CONTROL_MASTER_RIGHT);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    assert<uint8_t>::equals("Launch control master right: 0,0", currentClutch, (uint8_t)CLUTCH_NONE_VALUE);
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_FULL_VALUE);
    assert<uint8_t>::equals("Launch control master right: FULL,FULL", currentClutch, (uint8_t)CLUTCH_FULL_VALUE);
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_NONE_VALUE);
    assert<uint8_t>::equals("Launch control master right: FULL,0", currentClutch, (uint8_t)CLUTCH_DEFAULT_VALUE);
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_3_4_VALUE);
    assert<uint8_t>::equals("Launch control master right: FULL,3/4", currentClutch, (uint8_t)CLUTCH_3_4_VALUE);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_3_4_VALUE);
    assert<uint8_t>::equals("Launch control master right: 0, 3/4", currentClutch, (uint8_t)CLUTCH_3_4_VALUE);
}

void TG_analogClutchInButtonsMode()
{
    // Simulate operation of analog clutch paddles in buttons mode,
    // test that analog axes are not detected
    // test that buttons are detected
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::BUTTON);
    clutchPaddleType(true);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    assert<uint8_t>::equals("(1) clutch", (uint8_t)CLUTCH_NONE_VALUE, currentClutch);
    assert<uint8_t>::equals("(1) axis", (uint8_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assert<uint64_t>::equals("(1) state", 0ULL, currentState);
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_NONE_VALUE);
    assert<uint8_t>::equals("(2) clutch", (uint8_t)CLUTCH_NONE_VALUE, currentClutch);
    assert<uint8_t>::equals("(2) axis", (uint8_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assert<uint64_t>::equals("(2) state", BITMAP(LCLUTCH), currentState);
    input.axis(CLUTCH_FULL_VALUE, CLUTCH_FULL_VALUE);
    assert<uint8_t>::equals("(3) clutch", (uint8_t)CLUTCH_NONE_VALUE, currentClutch);
    assert<uint8_t>::equals("(3) axis", (uint8_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assert<uint64_t>::equals("(3) state", BITMAP(LCLUTCH) | BITMAP(RCLUTCH), currentState);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_FULL_VALUE);
    assert<uint8_t>::equals("(4) clutch", (uint8_t)CLUTCH_NONE_VALUE, currentClutch);
    assert<uint8_t>::equals("(4) axis", (uint8_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assert<uint64_t>::equals("(4) state", BITMAP(RCLUTCH), currentState);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    assert<uint8_t>::equals("(5) clutch", (uint8_t)CLUTCH_NONE_VALUE, currentClutch);
    assert<uint8_t>::equals("(5) axis", (uint8_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assert<uint64_t>::equals("(5) state", 0ULL, currentState);
}

void TG_digitalClutchInAxisMode()
{
    // Simulate operation of digital clutch paddles in axis mode,
    // test that analog axes are detected
    // test that buttons are not detected
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::AXIS);
    clutchPaddleType(false);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    input.release();
    assert<uint8_t>::equals("(1) left", (uint8_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assert<uint8_t>::equals("(1) right", (uint8_t)CLUTCH_NONE_VALUE, currentRightAxis);
    assert<uint64_t>::equals("(1) state", 0ULL, currentState);
    input.push(LCLUTCH);
    assert<uint8_t>::equals("(2) left", (uint8_t)CLUTCH_FULL_VALUE, currentLeftAxis);
    assert<uint8_t>::equals("(2) right", (uint8_t)CLUTCH_NONE_VALUE, currentRightAxis);
    assert<uint64_t>::equals("(2) state", 0ULL, currentState);
    input.push(RCLUTCH);
    assert<uint8_t>::equals("(3) left", (uint8_t)CLUTCH_FULL_VALUE, currentLeftAxis);
    assert<uint8_t>::equals("(3) right", (uint8_t)CLUTCH_FULL_VALUE, currentRightAxis);
    assert<uint64_t>::equals("(3) state", 0ULL, currentState);
    input.release(LCLUTCH);
    assert<uint8_t>::equals("(3) left", (uint8_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assert<uint8_t>::equals("(3) right", (uint8_t)CLUTCH_FULL_VALUE, currentRightAxis);
    assert<uint64_t>::equals("(3) state", 0ULL, currentState);
    input.release();
    assert<uint8_t>::equals("(4) left", (uint8_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assert<uint8_t>::equals("(4) right", (uint8_t)CLUTCH_NONE_VALUE, currentRightAxis);
    assert<uint64_t>::equals("(4) state", 0ULL, currentState);
    clutchPaddleType(true);
}

void TG_digitalClutchInButtonsMode()
{
    // Simulate operation of digital clutch paddles in buttons mode,
    // test that analog axes are not detected
    // test that buttons are detected
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::BUTTON);
    clutchPaddleType(false);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    input.release();
    assert<uint8_t>::equals("(1) clutch", (uint8_t)CLUTCH_NONE_VALUE, currentClutch);
    assert<uint8_t>::equals("(1) axis", (uint8_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assert<uint64_t>::equals("(1) state", 0ULL, currentState);
    input.push(LCLUTCH);
    assert<uint8_t>::equals("(2) clutch", (uint8_t)CLUTCH_NONE_VALUE, currentClutch);
    assert<uint8_t>::equals("(2) axis", (uint8_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assert<uint64_t>::equals("(2) state", BITMAP(LCLUTCH), currentState);
    input.push(RCLUTCH);
    assert<uint8_t>::equals("(3) clutch", (uint8_t)CLUTCH_NONE_VALUE, currentClutch);
    assert<uint8_t>::equals("(3) axis", (uint8_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assert<uint64_t>::equals("(3) state", BITMAP(RCLUTCH) | BITMAP(LCLUTCH), currentState);
    input.release(LCLUTCH);
    assert<uint8_t>::equals("(4) clutch", (uint8_t)CLUTCH_NONE_VALUE, currentClutch);
    assert<uint8_t>::equals("(4) axis", (uint8_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assert<uint64_t>::equals("(4) state", BITMAP(RCLUTCH), currentState);
    input.release();
}

void TG_analogClutchInAxisMode()
{
    // Simulate operation of analog clutch paddles in axis mode,
    // test that analog axes are detected,
    // test that combined clutch axis is not detected
    // test that buttons are not detected
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::AXIS);
    clutchPaddleType(true);
    input.release();
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    assert<uint8_t>::equals("(1) clutch", (uint8_t)CLUTCH_NONE_VALUE, currentClutch);
    assert<uint8_t>::equals("(1) left", (uint8_t)CLUTCH_NONE_VALUE, currentLeftAxis);
    assert<uint8_t>::equals("(1) right", (uint8_t)CLUTCH_NONE_VALUE, currentRightAxis);
    assert<uint64_t>::equals("(1) state", 0ULL, currentState);
    input.axis(CLUTCH_1_4_VALUE, CLUTCH_3_4_VALUE);
    assert<uint8_t>::equals("(2) clutch", (uint8_t)CLUTCH_NONE_VALUE, currentClutch);
    assert<uint8_t>::equals("(2) left", (uint8_t)CLUTCH_1_4_VALUE, currentLeftAxis);
    assert<uint8_t>::equals("(2) right", (uint8_t)CLUTCH_3_4_VALUE, currentRightAxis);
    assert<uint64_t>::equals("(2) state", 0ULL, currentState);
}

void TG_repeatedCommand()
{
    // Send input twice in a row without a real change in those inputs,
    // test that the corresponding command is not executed twice
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::CLUTCH);
    InputHubService::call::setAltButtonsWorkingMode(AltButtonsWorkingMode::ALT);
    input.axis(CLUTCH_NONE_VALUE, CLUTCH_NONE_VALUE);
    input.pushSeveral(BMP_CYCLE_ALT);
    assert<int>::equals(
        "Repeated input 1st",
        (int)AltButtonsWorkingMode::Regular,
        (int)InputHubService::call::getAltButtonsWorkingMode());
    input.repeat();
    assert<int>::equals(
        "Repeated input 2nd",
        (int)AltButtonsWorkingMode::Regular,
        (int)InputHubService::call::getAltButtonsWorkingMode());
    input.release();
}

void TG_securityLock()
{
    // initialize
    InputHubService::call::setSecurityLock(false);
    assert<bool>::equals("lock 1-1", false, InputHubService::call::getSecurityLock());

    // send input and check
    input.release();
    input.push(CMD);
    assert<bool>::equals("lock 1-2", false, InputHubService::call::getSecurityLock());
    input.push(OTHER);
    assert<bool>::equals("lock 1-3", false, InputHubService::call::getSecurityLock());
    input.push(UP);
    assert<bool>::equals("lock 2", true, InputHubService::call::getSecurityLock());
    input.release();
    input.push(CMD);
    input.push(OTHER);
    input.push(UP);
    input.release();
    assert<bool>::equals("lock 2", false, InputHubService::call::getSecurityLock());
}

void TG_neutralGear()
{
    // initialize
    input.release();
    assert<uint64_t>::equals("initialization", 0ULL, currentLow);
    assert<uint64_t>::equals("initialization", 0ULL, currentHigh);

    // press left shift paddle and check
    input.push(LSHIFT);
    assert<uint64_t>::equals("Lshift on", BITMAP(LSHIFT), currentLow);
    // press right shift paddle and check
    input.push(RSHIFT);
    assert<uint64_t>::equals("Rshift on", BITMAP(NEUTRAL), currentLow);
    // release right shift paddle and check
    input.release(RSHIFT);
    assert<uint64_t>::equals("Rshift off", 0ULL, currentLow);
    // release left shift paddle and check
    input.release(LSHIFT);
    assert<uint64_t>::equals("Lshift off", 0ULL, currentLow);

    // press left shift paddle again and check
    input.push(LSHIFT);
    assert<uint64_t>::equals("Lshift on + RShift off", BITMAP(LSHIFT), currentLow);
    // release left shift paddle, press right shift paddle and check
    input.release(LSHIFT);
    input.push(RSHIFT);
    assert<uint64_t>::equals("Rshift on + LShift off", BITMAP(RSHIFT), currentLow);
    // release all and check
    input.release(RSHIFT);
    assert<uint64_t>::equals("Lshift off + RShift off", 0ULL, currentLow);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
// Entry point
//------------------------------------------------------------------
//------------------------------------------------------------------

int main()
{
    InputNumber::bookAll();
    DeviceCapabilities::setFlag(DeviceCapability::CLUTCH_BUTTON);
    DeviceCapabilities::setFlag(DeviceCapability::CLUTCH_ANALOG);
    DeviceCapabilities::setFlag(DeviceCapability::DPAD);
    DeviceCapabilities::setFlag(DeviceCapability::ALT);
    inputHub::altButtons::inputs({ALT_IN});
    inputHub::altButtons::cycleWorkingModeInputs(COMBINATION_CYCLE_ALT);
    inputHub::dpad::inputs(UP, DOWN, LEFT, RIGHT);
    inputHub::dpad::cycleWorkingModeInputs(COMBINATION_CYCLE_DPAD);
    inputHub::clutch::inputs(LCLUTCH, RCLUTCH);
    inputHub::clutch::cycleWorkingModeInputs(COMBINATION_CYCLE_CLUTCH);
    inputHub::clutch::bitePointInputs(UP, DOWN);
    inputHub::securityLock::cycleWorkingModeInputs(COMBINATION_SECURITY_LOCK);
    inputHub::neutralGear::set(NEUTRAL, {LSHIFT, RSHIFT});
    internals::inputHub::getReady();
    OnStart::notify();

    assert<uint64_t>::equals("state at start", 0ULL, currentState);

    std::cout << ("- simulate ALT engagement with clutch paddles or buttons -") << std::endl;
    TG_altEngagement();

    std::cout << ("- simulate ALT button input when working mode is regular button -") << std::endl;
    TG_altInButtonsMode();

    std::cout << ("- simulate POV operation (valid input) -") << std::endl;
    TG_POV_validInput();

    std::cout << ("- simulate POV operation (invalid input) -") << std::endl;
    TG_POV_invalidInput();

    std::cout << ("- simulate POV operation while ALT pushed -") << std::endl;
    TG_POV_whileAlt();

    std::cout << ("- simulate POV operation in buttons mode -") << std::endl;
    TG_POV_ButtonsMode();

    std::cout << ("- simulate cycle ALT working mode -") << std::endl;
    TG_cycleAlt();

    std::cout << ("- simulate cycle clutch working mode -") << std::endl;
    TG_cycleClutchWorkingMode();

    std::cout << ("- simulate cycle DPAD working mode -") << std::endl;
    TG_cycleDPADWorkingMode();

    std::cout << ("- simulate non-mapped button combinations -") << std::endl;
    TG_nonMappedCombinations();

    std::cout << ("- simulate bite point calibration -") << std::endl;
    TG_bitePointCalibration();

    std::cout << ("- simulate bite point calibration in launch control mode -") << std::endl;
    TG_bitePointCalibrationInLaunchControl();

    std::cout << ("- simulate dual clutch operation -") << std::endl;
    TG_dualClutch();

    std::cout << ("- simulate launch control -") << std::endl;
    TG_launchControl();

    std::cout << ("- simulate analog clutch operation in buttons mode -") << std::endl;
    TG_analogClutchInButtonsMode();

    std::cout << ("- simulate digital clutch operation in axis mode -") << std::endl;
    TG_digitalClutchInAxisMode();

    std::cout << ("- simulate digital clutch operation in buttons mode -") << std::endl;
    TG_digitalClutchInButtonsMode();

    std::cout << ("- simulate analog clutch operation in axis mode -") << std::endl;
    TG_analogClutchInAxisMode();

    std::cout << ("- simulate repeated input without real change in inputs state -") << std::endl;
    TG_repeatedCommand();

    std::cout << ("- simulate security lock -") << std::endl;
    TG_securityLock();

    std::cout << ("- simulate neutral gear input -") << std::endl;
    TG_neutralGear();
}