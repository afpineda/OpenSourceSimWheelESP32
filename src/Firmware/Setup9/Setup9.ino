/**
 * @file Setup9.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-03-07
 * @brief Sim wheel setup #9
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.hpp"
#include "SimWheelUI.hpp"

//------------------------------------------------------------------
// Global customization
//------------------------------------------------------------------

/* -----------------------------------------------------------------
 >>>> [EN] DEVICE IDENTIFICATION
 >>>> [ES] IDENTIFICACIÓN DEL DISPOSITIVO
------------------------------------------------------------------ */

// [EN] Set a name for this device between double quotes
// [ES] Indique un nombre para este dispositivo entre comillas

std::string DEVICE_NAME = "Open steering wheel-9";

// [EN] Set a manufacturer's name for this device between double quotes
// [ES] Indique un nombre para el fabricante de este dispositivo entre comillas

std::string DEVICE_MANUFACTURER = "Mamandurrio";

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

#define USE_FUNKY true

#define ROT1_CW 33
#define ROT1_CCW ROT1_CW + 1
#define NAV_LEFT 50
#define NAV_RIGHT 51
#define NAV_UP 52
#define NAV_DOWN 53
#define CLUTCH1 60
#define CLUTCH2 61
#define ALT1 JOY_LTHUMBSTICK_CLICK
#define ALT2 JOY_RTHUMBSTICK_CLICK

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------

void simWheelSetup()
{
    inputs::addRotaryEncoder(GPIO_NUM_2, GPIO_NUM_3, ROT1_CW, ROT1_CCW);                        // ROT1
    inputs::addRotaryEncoder(GPIO_NUM_4, GPIO_NUM_5, ROT1_CW + 2, ROT1_CCW + 2);                // ROT2
    inputs::addRotaryEncoder(GPIO_NUM_12, GPIO_NUM_6, ROT1_CW + 4, ROT1_CCW + 4);               // ROT3
    inputs::addRotaryEncoder(GPIO_NUM_13, GPIO_NUM_7, ROT1_CW + 6, ROT1_CCW + 6);               // ROT4
    inputs::addRotaryEncoder(GPIO_NUM_11, GPIO_NUM_8, ROT1_CW + 8, ROT1_CCW + 8);               // ROT5
    inputs::addRotaryEncoder(GPIO_NUM_10, GPIO_NUM_9, ROT1_CW + 10, ROT1_CCW + 10);             // ROT6
    inputs::addRotaryEncoder(GPIO_NUM_37, GPIO_NUM_33, ROT1_CW + 12, ROT1_CCW + 12);            // ROT7
    inputs::addRotaryEncoder(GPIO_NUM_21, GPIO_NUM_16, ROT1_CW + 14, ROT1_CCW + 14, USE_FUNKY); // ROT8 or funky switch

    inputs::addButton(GPIO_NUM_1, JOY_A);
    inputs::addButton(GPIO_NUM_34, NAV_LEFT);
    inputs::addButton(GPIO_NUM_17, NAV_RIGHT);
    inputs::addButton(GPIO_NUM_38, NAV_UP);
    inputs::addButton(GPIO_NUM_18, NAV_DOWN);
    inputs::setAnalogClutchPaddles(GPIO_NUM_14, GPIO_NUM_15);

    MCP23017Expander chip1, chip2;

    chip1[MCP23017Pin::GPA0] = JOY_LSHIFT_PADDLE;
    chip1[MCP23017Pin::GPA1] = JOY_RSHIFT_PADDLE;
    chip1[MCP23017Pin::GPA2] = ALT1;
    chip1[MCP23017Pin::GPA3] = ALT2;
    chip1[MCP23017Pin::GPA4] = 10;
    chip1[MCP23017Pin::GPA5] = 11;
    chip1[MCP23017Pin::GPA6] = 12;
    chip1[MCP23017Pin::GPA7] = 13;
    chip1[MCP23017Pin::GPB0] = 14;
    chip1[MCP23017Pin::GPB1] = 15;
    chip1[MCP23017Pin::GPB2] = 16;
    chip1[MCP23017Pin::GPB3] = 17;
    chip1[MCP23017Pin::GPB4] = 18;
    chip1[MCP23017Pin::GPB5] = 19;
    chip1[MCP23017Pin::GPB6] = 20;
    chip1[MCP23017Pin::GPB7] = 21;

    chip2[MCP23017Pin::GPA0] = 22;
    chip2[MCP23017Pin::GPA1] = 23;
    chip2[MCP23017Pin::GPA2] = 24;
    chip2[MCP23017Pin::GPA3] = 25;
    chip2[MCP23017Pin::GPA4] = 26;
    chip2[MCP23017Pin::GPA5] = 27;
    chip2[MCP23017Pin::GPA6] = 28;
    chip2[MCP23017Pin::GPA7] = 29;
    chip2[MCP23017Pin::GPB0] = 30;
    chip2[MCP23017Pin::GPB1] = 31;
    chip2[MCP23017Pin::GPB2] = 32;
    chip2[MCP23017Pin::GPB3] = JOY_Y;
    chip2[MCP23017Pin::GPB4] = JOY_X;
    chip2[MCP23017Pin::GPB5] = JOY_B;
    chip2[MCP23017Pin::GPB6] = JOY_BACK;
    chip2[MCP23017Pin::GPB7] = JOY_START;

    inputs::addMCP23017Expander(chip1, 7);
    inputs::addMCP23017Expander(chip2, 0);

    inputHub::clutch::inputs(CLUTCH1, CLUTCH2);
    inputHub::dpad::inputs(NAV_UP, NAV_DOWN, NAV_LEFT, NAV_RIGHT);
    inputHub::altButtons::inputs({(ALT1), (ALT2)});
    inputHub::clutch::bitePointInputs(ROT1_CW, ROT1_CCW); // Rotary 1
    inputHub::clutch::cmdRecalibrateAxisInputs({(JOY_LSHIFT_PADDLE), (JOY_RSHIFT_PADDLE), (JOY_START)});
    inputHub::clutch::cycleWorkingModeInputs({(JOY_START), (JOY_LSHIFT_PADDLE)});
    inputHub::altButtons::cycleWorkingModeInputs({(JOY_START), (JOY_RSHIFT_PADDLE)});
}

//------------------------------------------------------------------

void customFirmware()
{
    simWheelSetup();
    hid::configure(
        DEVICE_NAME,
        DEVICE_MANUFACTURER,
        false);
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    firmware::run(customFirmware);
}

void loop()
{
    vTaskDelay(portMAX_DELAY);
}
