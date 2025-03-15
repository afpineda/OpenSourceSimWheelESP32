/**
 * @file Setup1.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-01-13
 * @brief Sim wheel setup #1
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.hpp"
#include "SimWheelUI.hpp"

//------------------------------------------------------------------
// Global customization
//------------------------------------------------------------------

#define IN_UP 30
#define IN_DOWN 31
#define IN_LEFT 32
#define IN_RIGHT 33
#define IN_ALT1 20
#define IN_ALT2 21

/* -----------------------------------------------------------------
 >>>> [EN] DEVICE IDENTIFICATION
 >>>> [ES] IDENTIFICACIÓN DEL DISPOSITIVO
------------------------------------------------------------------ */

std::string DEVICE_NAME = "Open steering wheel-1";
std::string DEVICE_MANUFACTURER = "Mamandurrio";

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------

void simWheelSetup()
{
    ButtonMatrix mtx;
    mtx[GPIO_NUM_4][GPIO_NUM_15] = JOY_A;
    mtx[GPIO_NUM_16][GPIO_NUM_15] = JOY_B;
    mtx[GPIO_NUM_17][GPIO_NUM_15] = JOY_X;
    mtx[GPIO_NUM_1][GPIO_NUM_15] = JOY_Y;
    mtx[GPIO_NUM_23][GPIO_NUM_15] = JOY_BACK;

    mtx[GPIO_NUM_4][GPIO_NUM_5] = JOY_START;
    mtx[GPIO_NUM_16][GPIO_NUM_5] = 8;
    mtx[GPIO_NUM_17][GPIO_NUM_5] = 9;
    mtx[GPIO_NUM_1][GPIO_NUM_5] = 10;
    mtx[GPIO_NUM_23][GPIO_NUM_5] = 11;

    mtx[GPIO_NUM_4][GPIO_NUM_21] = 12;
    mtx[GPIO_NUM_16][GPIO_NUM_21] = 13;
    mtx[GPIO_NUM_17][GPIO_NUM_21] = 14;
    mtx[GPIO_NUM_1][GPIO_NUM_21] = 15;
    mtx[GPIO_NUM_23][GPIO_NUM_21] = 16;

    mtx[GPIO_NUM_4][GPIO_NUM_22] = 17;
    mtx[GPIO_NUM_16][GPIO_NUM_22] = IN_UP;
    mtx[GPIO_NUM_17][GPIO_NUM_22] = IN_DOWN;
    mtx[GPIO_NUM_1][GPIO_NUM_22] = IN_LEFT;
    mtx[GPIO_NUM_23][GPIO_NUM_22] = IN_RIGHT;

    mtx[GPIO_NUM_4][GPIO_NUM_13] = IN_ALT2;
    mtx[GPIO_NUM_16][GPIO_NUM_13] = IN_ALT1;
    mtx[GPIO_NUM_17][GPIO_NUM_13] = JOY_RTHUMBSTICK_CLICK;
    mtx[GPIO_NUM_1][GPIO_NUM_13] = JOY_LTHUMBSTICK_CLICK;
    mtx[GPIO_NUM_23][GPIO_NUM_13] = 63;
    inputs::addButtonMatrix(mtx);

    inputs::setAnalogClutchPaddles(GPIO_NUM_36, GPIO_NUM_39); // 18 and 19
    inputs::addRotaryEncoder(GPIO_NUM_34, GPIO_NUM_35, 40, 41);
    inputs::addRotaryEncoder(GPIO_NUM_32, GPIO_NUM_33, 42, 43);
    inputs::addRotaryEncoder(GPIO_NUM_25, GPIO_NUM_26, 44, 45);
    inputs::addRotaryEncoder(GPIO_NUM_27, GPIO_NUM_14, 46, 47);
    inputs::addRotaryEncoder(GPIO_NUM_19, GPIO_NUM_18, 48, 49, true); // ALPS

    inputHub::dpad::inputs(IN_UP, IN_DOWN, IN_LEFT, IN_RIGHT);
    inputHub::altButtons::inputs({IN_ALT1, IN_ALT2});
    inputHub::clutch::inputs(18, 19);
    inputHub::clutch::bitePointInputs(40, 41); // Rotary 1
    inputHub::clutch::cycleWorkingModeInputs({JOY_START, JOY_LB});
    inputHub::altButtons::cycleWorkingModeInputs({JOY_START, JOY_RB});
    inputHub::clutch::cmdRecalibrateAxisInputs({JOY_START, JOY_LB, JOY_RB});
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