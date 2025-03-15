/**
 * @file Proto2.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-04-21
 * @brief Integration test. See [Readme](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <HardwareSerial.h>
#include "Testing.hpp"
#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "SimWheelUI.hpp"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

#define LEFT_CLUTCH_IN 3
#define RIGHT_CLUTCH_IN 2
#define ALT_IN 10
#define CW_IN 12
#define CCW_IN 13
#define COMMAND_IN 7
#define CYCLE_CLUTCH_IN 6

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void customFirmware()
{
    power::configureWakeUp(TEST_ROTARY_SW);
    batteryMonitor::configure(
        TEST_BATTERY_READ,
        TEST_BATTERY_READ_ENABLE);
    internals::batteryMonitor::configureForTesting();
    pixels::configure(
        PixelGroup::GRP_TELEMETRY,
        TEST_D_OUT,
        8,
        TEST_LEVEL_SHIFTER,
        PixelDriver::WS2812,
        PixelFormat::AUTO,
        16);
    ui::addPixelControlNotifications();
    ui::add<SimpleShiftLight>(TEST_SIMPLE_SHIFT_LIGHT_PIN);

    ButtonMatrix mtx;
    setDebugInputNumbers(mtx);

    inputs::addButtonMatrix(mtx);
    inputs::addButton(TEST_ROTARY_SW, ALT_IN);
    inputs::addRotaryEncoder(
        TEST_ROTARY_CLK,
        TEST_ROTARY_DT,
        CW_IN,
        CCW_IN,
        false);

    inputHub::clutch::inputs(LEFT_CLUTCH_IN, RIGHT_CLUTCH_IN);
    inputHub::clutch::cycleWorkingModeInputs({(COMMAND_IN), (CYCLE_CLUTCH_IN)});
    inputHub::clutch::bitePointInputs(CW_IN, CCW_IN);

    hid::configure("Proto2", "Mamandurrio", true);
}

void setup()
{
    Serial.begin(115200);
    Serial.println("-- READY --");
    firmware::run(customFirmware);
    Serial.println("-- GO --");
}

void loop()
{
    vTaskDelay(portMAX_DELAY);
}