/**
 * @file Proto1.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-01
 * @brief Integration test.
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <HardwareSerial.h>
#include "Testing.hpp"
#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

#define LEFT_CLUTCH_IN 0
#define RIGHT_CLUTCH_IN 1
#define ALT_IN 10
#define CW_IN 13
#define CCW_IN 12
#define COMMAND_IN 2
#define CYCLE_ALT_IN 3
#define CYCLE_CLUTCH_IN 4
#define CYCLE_DPAD_IN 5
#define UP 3
#define DOWN 4
#define LEFT 5
#define RIGHT 6

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

void internals::batteryCalibration::getReady() {}
void internals::batteryMonitor::getReady() {}
void internals::power::getReady() {}
void internals::ui::getReady() {}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void customFirmware()
{
    ButtonMatrix mtx;
    setDebugInputNumbers(mtx);
    inputs::addButtonMatrix(mtx);
    inputs::addButton(TEST_ROTARY_SW, ALT_IN);
    inputs::addRotaryEncoder(TEST_ROTARY_CLK, TEST_ROTARY_DT, CW_IN, CCW_IN, false);
    inputs::setAnalogClutchPaddles(TEST_ANALOG_PIN1, TEST_ANALOG_PIN2);

    inputHub::altButtons::inputs({ALT_IN});
    inputHub::altButtons::cycleWorkingModeInputs({(COMMAND_IN), (CYCLE_ALT_IN)});

    inputHub::securityLock::cycleWorkingModeInputs({(COMMAND_IN), RIGHT});

    inputHub::clutch::cycleWorkingModeInputs({(COMMAND_IN), (CYCLE_CLUTCH_IN)});
    inputHub::clutch::inputs(LEFT_CLUTCH_IN, RIGHT_CLUTCH_IN);
    inputHub::clutch::bitePointInputs(CW_IN, CCW_IN);

    inputHub::dpad::cycleWorkingModeInputs({(COMMAND_IN), (CYCLE_DPAD_IN)});
    inputHub::dpad::inputs(UP, DOWN, LEFT, RIGHT);

    hid::configure("Proto1", "Mamandurrio", false);
}

void setup()
{
    Serial.begin(115200);
    Serial.println("-- READY --");
    firmware::run(customFirmware);
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::CLUTCH, false);
    InputHubService::call::setAltButtonsWorkingMode(AltButtonsWorkingMode::ALT, false);
    InputHubService::call::setDPadWorkingMode(DPadWorkingMode::Navigation, false);
    InputHubService::call::setBitePoint(CLUTCH_DEFAULT_VALUE, false);
    InputHubService::call::setSecurityLock(false, false);
    Serial.println("-- GO --");
}

void loop()
{
    vTaskDelay(portMAX_DELAY);
}