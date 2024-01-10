/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "SimWheel.h"
#include "debugUtils.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

// #define LCLUTCH 1
// #define RCLUTCH 2
#define ALT 3
#define CMD 4
#define CYCLE_CLUTCH 5
#define CYCLE_ALT 6
#define UP 7
#define DOWN 8
#define LEFT 9
#define RIGHT 10

#define BMP_CYCLE_CLUTCH BITMAP(CMD) | BITMAP(CYCLE_CLUTCH)
#define BMP_CYCLE_ALT BITMAP(CMD) | BITMAP(CYCLE_ALT)
#define BMP_SELECT_CLUTCH_F BITMAP(CMD) | BITMAP(UP)
#define BMP_SELECT_ALT_F BITMAP(CMD) | BITMAP(DOWN)
#define BMP_SELECT_AXIS_F BITMAP(CMD) | BITMAP(LEFT)
#define BMP_SELECT_BUTTON_F BITMAP(CMD) | BITMAP(RIGHT)

#define ALT_B BITMAP(ALT)
#define UP_B BITMAP(UP)
#define DOWN_B BITMAP(DOWN)
#define LEFT_B BITMAP(LEFT)
#define RIGHT_B BITMAP(RIGHT)

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

uint8_t currentPOV = 0;
bool currentALTEnabled = false;
inputBitmap_t currentState = 0;

void hidImplementation::reset()
{
    currentPOV = 0;
    currentALTEnabled = false;
    currentState = 0;
    //Serial.println("hidImplementation::reset()");
}

void hidImplementation::reportInput(
    inputBitmap_t globalState,
    bool altEnabled,
    uint8_t POVstate)
{
    currentState = globalState;
    currentPOV = POVstate;
    currentALTEnabled = altEnabled;
    // Serial.println(".");
}

void notify::bitePoint(clutchValue_t value)
{
    // Serial.print("Bite point: ");
    // Serial.println(value);
}

void hidImplementation::reportChangeInConfig()
{
}

void capabilities::setFlag(deviceCapability_t a, bool b)
{
}

void inputs::recalibrateAxes()
{

}

void batteryCalibration::restartAutoCalibration()
{

}

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

void push(inputBitmap_t bmp)
{
    inputHub::onStateChanged(bmp, bmp);
}

void release(inputBitmap_t bmp)
{
    inputHub::onStateChanged(0, bmp);
}

template <typename T>
void assertEquals(const char *text, T expected, T found)
{
    if (expected != found)
    {
        serialPrintf("[assertEquals] (%s). Expected: %d, found: %d\n", text, expected, found);
    }
}

template <typename T>
void pushAssertEqualsRelease(inputBitmap_t bmp, const char *text, T expected, T *found)
{
    inputHub::onStateChanged(bmp, bmp);
    assertEquals<T>(text, expected, *found);
    inputHub::onStateChanged(0, bmp);
}

void inputs::update()
{

}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    clutchValue_t biteP;

    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("-- READY --");

    inputHub::setALTButton(ALT);
    inputHub::setDPADControls(UP, DOWN, LEFT, RIGHT);
    inputHub::cycleALTButtonsWorkingMode_setBitmap(BMP_CYCLE_ALT);
    inputHub::cycleCPWorkingMode_setBitmap(BMP_CYCLE_CLUTCH);
    inputHub::cpWorkingMode_setBitmaps(
        BMP_SELECT_CLUTCH_F,
        BMP_SELECT_AXIS_F,
        BMP_SELECT_ALT_F,
        BMP_SELECT_BUTTON_F);
    inputHub::setClutchCalibrationButtons(UP, DOWN);

    Serial.println("-- GO --");
    assertEquals<inputBitmap_t>("state at start", 0, currentState);

    Serial.println("- simulate POV operation (valid input) -");
    pushAssertEqualsRelease<uint8_t>(UP_B, "UP_B", 1, &currentPOV);
    pushAssertEqualsRelease<uint8_t>(DOWN_B, "DOWN_B", 5, &currentPOV);
    pushAssertEqualsRelease<uint8_t>(LEFT_B, "LEFT_B", 7, &currentPOV);
    pushAssertEqualsRelease<uint8_t>(RIGHT_B, "RIGHT_B", 3, &currentPOV);
    pushAssertEqualsRelease<uint8_t>(UP_B | LEFT_B, "UP_B | LEFT_B", 8, &currentPOV);
    pushAssertEqualsRelease<uint8_t>(DOWN_B | LEFT_B, "DOWN_B | LEFT_B", 6, &currentPOV);
    pushAssertEqualsRelease<uint8_t>(UP_B | RIGHT_B, "UP_B | RIGHT_B", 2, &currentPOV);
    pushAssertEqualsRelease<uint8_t>(DOWN_B | RIGHT_B, "DOWN_B | RIGHT_B", 4, &currentPOV);
    push(UP_B);
    assertEquals<inputBitmap_t>("state at push", 0, currentState);
    release(UP_B);
    assertEquals<inputBitmap_t>("state at release", 0, currentState);

    Serial.println("- simulate POV operation (invalid input) -");
    pushAssertEqualsRelease<uint8_t>(UP_B | DOWN_B, "UP_B | DOWN_B", 0, &currentPOV);
    pushAssertEqualsRelease<uint8_t>(LEFT_B | RIGHT_B, "LEFT_B | RIGHT_B", 0, &currentPOV);
    push(UP_B | DOWN_B);
    assertEquals<inputBitmap_t>("state at push", 0, currentState);
    release(UP_B | DOWN_B);
    assertEquals<inputBitmap_t>("state at release", 0, currentState);

    Serial.println("- simulate POV operation while ALT pushed -");
    userSettings::setALTButtonsWorkingMode(true);
    push(ALT_B);
    assertEquals<inputBitmap_t>("ALT filtered state", 0, currentState);
    push(ALT_B | UP_B);
    assertEquals<uint8_t>("UP_B | ALT_B POV", 0, currentPOV);
    assertEquals<inputBitmap_t>("UP_B | ALT_B state at push", UP_B, currentState);
    assertEquals<bool>("UP_B | ALT_B altEnabled", true, currentALTEnabled);
    release(UP_B);
    assertEquals<inputBitmap_t>("UP_B | ALT_B state at release", 0, currentState);
    pushAssertEqualsRelease<uint8_t>(DOWN_B | ALT_B, "DOWN_B | ALT_B", 0, &currentPOV);
    pushAssertEqualsRelease<uint8_t>(LEFT_B | ALT_B, "LEFT_B | ALT_B", 0, &currentPOV);
    pushAssertEqualsRelease<uint8_t>(RIGHT_B | ALT_B, "RIGHT_B | ALT_B", 0, &currentPOV);
    release(ALT_B);

    Serial.println("- simulate cycle ALT function -");
    userSettings::setALTButtonsWorkingMode(true);
    pushAssertEqualsRelease<bool>(BMP_CYCLE_ALT, "Cycle alt 1", false, (bool *)&userSettings::altModeForAltButtons);
    pushAssertEqualsRelease<bool>(BMP_CYCLE_ALT, "Cycle alt 2", true, (bool *)&userSettings::altModeForAltButtons);

    Serial.println("- simulate cycle clutch function -");
    userSettings::setCPWorkingMode(CF_BUTTON);
    pushAssertEqualsRelease<clutchFunction_t>(BMP_CYCLE_CLUTCH, "Cycle clutch 1", CF_CLUTCH, (clutchFunction_t *)&userSettings::currentFunction);
    pushAssertEqualsRelease<clutchFunction_t>(BMP_CYCLE_CLUTCH, "Cycle clutch 2", CF_AXIS, (clutchFunction_t *)&userSettings::currentFunction);
    pushAssertEqualsRelease<clutchFunction_t>(BMP_CYCLE_CLUTCH, "Cycle clutch 3", CF_ALT, (clutchFunction_t *)&userSettings::currentFunction);
    pushAssertEqualsRelease<clutchFunction_t>(BMP_CYCLE_CLUTCH, "Cycle clutch 4", CF_BUTTON, (clutchFunction_t *)&userSettings::currentFunction);

    Serial.println("- simulate explicit selection of clutch function -");
    userSettings::setCPWorkingMode(CF_BUTTON);
    pushAssertEqualsRelease<clutchFunction_t>(BMP_SELECT_ALT_F, "CF_ALT", CF_ALT, (clutchFunction_t *)&userSettings::currentFunction);
    userSettings::setCPWorkingMode(CF_BUTTON);
    pushAssertEqualsRelease<clutchFunction_t>(BMP_SELECT_CLUTCH_F, "CF_CLUTCH", CF_CLUTCH, (clutchFunction_t *)&userSettings::currentFunction);
    userSettings::setCPWorkingMode(CF_ALT);
    pushAssertEqualsRelease<clutchFunction_t>(BMP_SELECT_BUTTON_F, "CF_BUTTON", CF_BUTTON, (clutchFunction_t *)&userSettings::currentFunction);
    userSettings::setCPWorkingMode(CF_BUTTON);
    pushAssertEqualsRelease<clutchFunction_t>(BMP_SELECT_AXIS_F, "CF_AXIS", CF_AXIS, (clutchFunction_t *)&userSettings::currentFunction);

    Serial.println("- simulate non-mapped button combinations -");
    userSettings::setCPWorkingMode(CF_BUTTON);
    userSettings::setALTButtonsWorkingMode(true);
    push(BMP_CYCLE_ALT | BMP_CYCLE_CLUTCH);
    assertEquals<clutchFunction_t>("CF_BUTTON", CF_BUTTON, userSettings::currentFunction);
    assertEquals<bool>("alt mode", true, userSettings::altModeForAltButtons);
    release(BMP_CYCLE_ALT | BMP_CYCLE_CLUTCH);

    Serial.println("- simulate bite point calibration -");
    userSettings::setCPWorkingMode(CF_CLUTCH);
    userSettings::setBitePoint(CLUTCH_DEFAULT_VALUE);
    userSettings::setLeftAxis(CLUTCH_FULL_VALUE);
    userSettings::setRightAxis(CLUTCH_NONE_VALUE);
    biteP = userSettings::bitePoint;
    push(UP_B);
    release(UP_B);
    if (userSettings::bitePoint <= biteP)
        serialPrintf("Invalid bite point. Expected > %d, Found: %d\n", biteP, userSettings::bitePoint);
    biteP = userSettings::bitePoint;
    push(DOWN_B);
    release(DOWN_B);
    push(DOWN_B);
    release(DOWN_B);
    if (userSettings::bitePoint >= biteP)
        serialPrintf("Invalid bite point. Expected < %d, Found: %d\n", biteP, userSettings::bitePoint);

    Serial.println("-- END --");
    for (;;)
        ;
}

void loop()
{
}