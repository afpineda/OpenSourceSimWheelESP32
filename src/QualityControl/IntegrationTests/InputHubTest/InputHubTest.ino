/**
 * @file InputHubTest.ino

 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-03-10
 * @brief Integration test.
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"

#include <string>

#include <HardwareSerial.h>
#include "freertos/FreeRTOS.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

FakeInput fakeInput;

#define LEFT_CLUTCH_IN 0
#define RIGHT_CLUTCH_IN 1

#define COMMAND_IN 2
#define CYCLE_ALT_IN 3
#define CYCLE_CLUTCH_IN 4
#define CYCLE_DPAD_IN 5

#define ALT_IN 10
#define CW_IN 12
#define CCW_IN 13
#define DPAD_LEFT_IN 14
#define DPAD_RIGHT_IN 15
#define DPAD_UP_IN 16
#define DPAD_DOWN_IN 17

#define WAIT_TICKS pdMS_TO_TICKS(120)

#define BMP(n) (1ULL << (n))

#define DPAD_CENTERED_STATE 0
#define DPAD_UP_STATE 1
#define DPAD_UP_RIGHT_STATE 2
#define DPAD_RIGHT_STATE 3
#define DPAD_DOWN_RIGHT_STATE 4
#define DPAD_DOWN_STATE 5
#define DPAD_DOWN_LEFT_STATE 6
#define DPAD_LEFT_STATE 7
#define DPAD_UP_LEFT_STATE 8

uint64_t _inputsLow;
uint64_t _inputsHigh;
uint8_t _POVstate;
uint8_t _leftAxis;
uint8_t _rightAxis;
uint8_t _clutchAxis;

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

bool internals::hid::isConnected() { return true; }
bool internals::hid::supportsCustomHardwareID() { return false; }
void internals::hid::reportChangeInConfig() {}
void internals::hid::reportBatteryLevel(int batteryLevel) {}

void internals::hid::reset()
{
    _inputsLow = 0ULL;
    _inputsHigh = 0ULL;
    _POVstate = 0;
    _leftAxis = 0;
    _rightAxis = 0;
    _clutchAxis = 0;
}

void internals::hid::begin(
    std::string deviceName,
    std::string deviceManufacturer,
    bool enableAutoPowerOff,
    uint16_t vendorID,
    uint16_t productID) {}

void internals::hid::reportInput(
    uint64_t inputsLow,
    uint64_t inputsHigh,
    uint8_t POVstate,
    uint8_t leftAxis,
    uint8_t rightAxis,
    uint8_t clutchAxis)
{
    _inputsLow = inputsLow;
    _inputsHigh = inputsHigh;
    _POVstate = POVstate;
    _leftAxis = leftAxis;
    _rightAxis = rightAxis;
    _clutchAxis = clutchAxis;
}

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

void checkInputs(
    uint64_t inputsLow,
    uint64_t inputsHigh,
    std::string msg)
{
    vTaskDelay(WAIT_TICKS);
    if (inputsLow != _inputsLow)
    {
        Serial.println("MISMATCH in inputsLow");
        Serial.printf("Expected: %llu, found: %llu\n", inputsLow, _inputsLow);
        Serial.print("At: ");
        Serial.println(msg.c_str());
    }
    if (inputsHigh != _inputsHigh)
    {
        Serial.println("MISMATCH in inputsHigh");
        Serial.printf("Expected: %llu, found: %llu\n", inputsHigh, _inputsHigh);
        Serial.print("At: ");
        Serial.println(msg.c_str());
    }
}

void checkAxes(
    uint8_t leftAxis,
    uint8_t rightAxis,
    uint8_t clutchAxis,
    std::string msg)
{
    vTaskDelay(WAIT_TICKS);
    if (leftAxis != _leftAxis)
    {
        Serial.println("MISMATCH in left axis");
        Serial.printf("Expected: %hhu, found: %hhu\n", leftAxis, _leftAxis);
        Serial.print("At: ");
        Serial.println(msg.c_str());
    }
    if (rightAxis != _rightAxis)
    {
        Serial.println("MISMATCH in right axis");
        Serial.printf("Expected: %hhu, found: %hhu\n", rightAxis, _rightAxis);
        Serial.print("At: ");
        Serial.println(msg.c_str());
    }
    if ((_clutchAxis < (clutchAxis - 1)) || (_clutchAxis > (clutchAxis + 1)))
    {
        Serial.println("MISMATCH in clutch axis");
        Serial.printf("Expected: %hhu, found: %hhu\n", clutchAxis, _clutchAxis);
        Serial.print("At: ");
        Serial.println(msg.c_str());
    }
}

void checkDpad(uint8_t POVstate, std::string msg)
{
    vTaskDelay(WAIT_TICKS);
    if (POVstate != _POVstate)
    {
        Serial.println("MISMATCH in DPAD");
        Serial.printf("Expected: %hhu, found: %hhu\n", POVstate, _POVstate);
        Serial.print("At: ");
        Serial.println(msg.c_str());
    }
}

//------------------------------------------------------------------
// Test groups
//------------------------------------------------------------------

void TG_filteredInput()
{
    fakeInput.clear();
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::CLUTCH);
    InputHubService::call::setAltButtonsWorkingMode(AltButtonsWorkingMode::ALT);

    fakeInput.clear();
    fakeInput.press(45);
    checkInputs(BMP(45), 0ULL, "Not to be filtered");

    fakeInput.clear();
    fakeInput.press(ALT_IN);
    checkInputs(0ULL, 0ULL, "ALT pressed");

    fakeInput.clear();
    fakeInput.press(COMMAND_IN);
    fakeInput.press(CYCLE_CLUTCH_IN);
    checkInputs(0ULL, 0ULL, "Cycle clutch working mode");

    fakeInput.clear();
    fakeInput.press(COMMAND_IN);
    fakeInput.press(CYCLE_ALT_IN);
    checkInputs(0ULL, 0ULL, "Cycle ALT working mode");
}

void TG_ClutchMode()
{
    fakeInput.clear();
    InputHubService::call::setBitePoint(CLUTCH_DEFAULT_VALUE);
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::CLUTCH);
    uint8_t bp;

    fakeInput.leftAxis = CLUTCH_FULL_VALUE;
    bp = InputHubService::call::getBitePoint();
    checkAxes(0, 0, bp, "Left clutch paddle");

    fakeInput.leftAxis = 0;
    fakeInput.rightAxis = CLUTCH_FULL_VALUE;
    vTaskDelay(WAIT_TICKS);
    bp = InputHubService::call::getBitePoint();
    checkAxes(0, 0, bp, "Right clutch paddle");

    fakeInput.press(CW_IN);
    fakeInput.release(CW_IN);
    vTaskDelay(WAIT_TICKS);
    bp = InputHubService::call::getBitePoint();
    checkAxes(0, 0, bp, "Increase BP");

    fakeInput.press(CCW_IN);
    fakeInput.release(CCW_IN);
    vTaskDelay(WAIT_TICKS);
    bp = InputHubService::call::getBitePoint();
    checkAxes(0, 0, bp, "Decrease BP");
}

void TG_AxesMode()
{
    fakeInput.clear();
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::AXIS);

    fakeInput.leftAxis = 12;
    fakeInput.rightAxis = 132;
    checkAxes(12, 132, 0, "Individual axes");
}

void TG_ClutchButtonsMode()
{
    fakeInput.clear();
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::BUTTON);

    fakeInput.leftAxis = CLUTCH_1_4_VALUE;
    fakeInput.rightAxis = CLUTCH_1_4_VALUE;
    checkInputs(0ULL, 0ULL, "Buttons mode: 1/4");

    fakeInput.leftAxis = CLUTCH_FULL_VALUE;
    fakeInput.rightAxis = CLUTCH_FULL_VALUE;
    checkInputs(BMP(LEFT_CLUTCH_IN) | BMP(RIGHT_CLUTCH_IN), 0ULL, "Buttons mode: 4/4");

    fakeInput.leftAxis = CLUTCH_3_4_VALUE;
    fakeInput.rightAxis = CLUTCH_3_4_VALUE;
    checkInputs(BMP(LEFT_CLUTCH_IN) | BMP(RIGHT_CLUTCH_IN), 0ULL, "Buttons mode: 3/4");

    fakeInput.leftAxis = CLUTCH_NONE_VALUE;
    fakeInput.rightAxis = CLUTCH_NONE_VALUE;
    checkInputs(0ULL, 0ULL, "Buttons mode: 0/4");
}

void TG_ClutchAltMode()
{
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::ALT);
    InputHubService::call::setAltButtonsWorkingMode(AltButtonsWorkingMode::Regular);

    fakeInput.clear();
    fakeInput.leftAxis = CLUTCH_3_4_VALUE;
    fakeInput.press(45);
    checkInputs(0ULL, BMP(45), "Left clutch in alternate mode");
    checkAxes(0, 0, 0, "Left clutch in alternate mode");
    fakeInput.clear();
    vTaskDelay(WAIT_TICKS);

    fakeInput.rightAxis = CLUTCH_3_4_VALUE;
    fakeInput.press(45);
    checkInputs(0ULL, BMP(45), "Right clutch in alternate mode");
    checkAxes(0, 0, 0, "Right clutch in alternate mode");
    fakeInput.clear();
}

void TG_LaunchControlMode()
{
    uint8_t bp;
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::LAUNCH_CONTROL_MASTER_LEFT);
    InputHubService::call::setBitePoint(CLUTCH_DEFAULT_VALUE);
    fakeInput.clear();
    vTaskDelay(WAIT_TICKS);

    fakeInput.rightAxis = CLUTCH_1_4_VALUE;
    checkAxes(0, 0, 0, "Slave 1/4");

    fakeInput.rightAxis = CLUTCH_FULL_VALUE;
    bp = InputHubService::call::getBitePoint();
    checkAxes(0, 0, bp, "Slave 3/4");

    fakeInput.leftAxis = CLUTCH_1_4_VALUE;
    checkAxes(0, 0, bp, "Slave 3/4 + master 1/4");

    fakeInput.leftAxis = CLUTCH_FULL_VALUE;
    checkAxes(0, 0, CLUTCH_FULL_VALUE, "Slave 3/4 + master 4/4");

    fakeInput.rightAxis = CLUTCH_NONE_VALUE;
    fakeInput.leftAxis = CLUTCH_1_4_VALUE;
    checkAxes(0, 0, CLUTCH_1_4_VALUE, "Slave 0/4 + master 1/4");

    fakeInput.clear();
    fakeInput.rightAxis = CLUTCH_FULL_VALUE;
    fakeInput.press(CW_IN);
    vTaskDelay(WAIT_TICKS);
    bp = InputHubService::call::getBitePoint();
    checkAxes(0, 0, bp, "Slave + bite point increase");
}

void TG_AltButton()
{
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::CLUTCH);
    InputHubService::call::setAltButtonsWorkingMode(AltButtonsWorkingMode::ALT);

    fakeInput.clear();
    fakeInput.press(ALT_IN);
    fakeInput.press(45);
    checkInputs(0ULL, BMP(45), "Alt button in alternate mode");
    fakeInput.release(45);
    fakeInput.release(ALT_IN);
    vTaskDelay(WAIT_TICKS);

    InputHubService::call::setAltButtonsWorkingMode(AltButtonsWorkingMode::Regular);

    fakeInput.press(ALT_IN);
    fakeInput.press(45);
    checkInputs(BMP(45) | BMP(ALT_IN), 0ULL, "Alt button in regular mode");
}

void TG_Dpad()
{
    fakeInput.clear();
    InputHubService::call::setDPadWorkingMode(DPadWorkingMode::Navigation);

    fakeInput.press(DPAD_UP_IN);
    checkDpad(DPAD_UP_STATE, "DPAD up");

    fakeInput.press(DPAD_LEFT_IN);
    checkDpad(DPAD_UP_LEFT_STATE, "DPAD up+left");

    fakeInput.release(DPAD_LEFT_IN);
    fakeInput.press(DPAD_RIGHT_IN);
    checkDpad(DPAD_UP_RIGHT_STATE, "DPAD up+right");

    fakeInput.clear();
    fakeInput.press(DPAD_DOWN_IN);
    checkDpad(DPAD_DOWN_STATE, "DPAD down");

    fakeInput.press(DPAD_LEFT_IN);
    checkDpad(DPAD_DOWN_LEFT_STATE, "DPAD down+left");

    fakeInput.release(DPAD_LEFT_IN);
    fakeInput.press(DPAD_RIGHT_IN);
    checkDpad(DPAD_DOWN_RIGHT_STATE, "DPAD down+right");

    fakeInput.clear();
    fakeInput.press(DPAD_LEFT_IN);
    checkDpad(DPAD_LEFT_STATE, "DPAD left");

    fakeInput.clear();
    fakeInput.press(DPAD_RIGHT_IN);
    checkDpad(DPAD_RIGHT_STATE, "DPAD right");

    fakeInput.clear();
    checkDpad(DPAD_CENTERED_STATE, "DPAD center");
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    try
    {
        InputNumber::bookAll();
        internals::inputs::addFakeInput(&fakeInput);
        inputHub::altButtons::inputs({ALT_IN});
        inputHub::altButtons::cycleWorkingModeInputs({COMMAND_IN, CYCLE_ALT_IN});
        inputHub::clutch::inputs(LEFT_CLUTCH_IN, RIGHT_CLUTCH_IN);
        inputHub::clutch::cycleWorkingModeInputs({COMMAND_IN, CYCLE_CLUTCH_IN});
        inputHub::clutch::bitePointInputs(CW_IN, CCW_IN);
        inputHub::dpad::inputs(DPAD_UP_IN, DPAD_DOWN_IN, DPAD_LEFT_IN, DPAD_RIGHT_IN);
        inputHub::dpad::cycleWorkingModeInputs({COMMAND_IN, CYCLE_DPAD_IN});

        internals::inputs::getReady();
        internals::inputHub::getReady();
        internals::hid::common::getReady();
        internals::inputMap::getReady();
        OnStart::notify();

        Serial.println("-- GO --");

        TG_filteredInput();
        TG_ClutchMode();
        TG_AxesMode();
        TG_ClutchAltMode();
        TG_ClutchButtonsMode();
        TG_LaunchControlMode();
        TG_AltButton();
        TG_Dpad();
    }
    catch (std::exception &e)
    {
        Serial.println("EXCEPTION:");
        Serial.println(e.what());
        for (;;)
            ;
    }
    Serial.println("-- DONE --");
    Serial.println("(reset required)");
}

void loop()
{
}