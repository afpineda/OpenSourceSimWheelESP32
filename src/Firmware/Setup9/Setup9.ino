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

#include "SimWheel.h"

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

#define ROT1_CW 31
#define ROT1_CCW ROT1_CW + 1
#define NAV_LEFT 50
#define NAV_RIGHT 51
#define NAV_UP 52
#define NAV_DOWN 53
#define CLUTCH1 60
#define CLUTCH2 61
#define ALT1 JOY_LTHUMBSTICK_CLICK
#define ALT2 JOY_RTHUMBSTICK_CLICK

static const inputNumber_t mcpNumbers_addr7[] = {
    JOY_LSHIFT_PADDLE, // Index 0
    JOY_RSHIFT_PADDLE,
    ALT1,
    ALT2,
    JOY_LTHUMBSTICK_CLICK,
    JOY_RTHUMBSTICK_CLICK,
    10,
    11,
    12, // Index 8
    13,
    14,
    15,
    16,
    17,
    18,
    19};

static const inputNumber_t mcpNumbers_addr0[] = {
    20, // Index 0
    21,
    22,
    23,
    24,
    25,
    26,
    27,
    28, // Index 8
    29,
    30,
    JOY_Y, // Index 11
    JOY_X,
    JOY_B,
    JOY_BACK,
    JOY_START};

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
    inputs::addDigital(GPIO_NUM_33, true, true, JOY_A);
    inputs::addDigital(GPIO_NUM_34, true, true, NAV_LEFT);
    inputs::addDigital(GPIO_NUM_17, true, true, NAV_RIGHT);
    inputs::addDigital(GPIO_NUM_38, true, true, NAV_UP);
    inputs::addDigital(GPIO_NUM_18, true, true, NAV_DOWN);
    inputs::setAnalogClutchPaddles(GPIO_NUM_14, GPIO_NUM_15);
    inputs::addMCP23017Digital(mcpNumbers_addr7, 7, false);
    inputs::addMCP23017Digital(mcpNumbers_addr0, 0, false);

    inputHub::setClutchInputNumbers(CLUTCH1, CLUTCH2);
    inputHub::setDPADControls(NAV_UP, NAV_DOWN, NAV_LEFT, NAV_RIGHT);
    inputHub::setALTBitmap(BITMAP(ALT1) | BITMAP(ALT2));
    inputHub::setClutchCalibrationButtons(ROT1_CW, ROT1_CCW); // Rotary 1
    inputHub::cmdRecalibrateAnalogAxis_setBitmap(BITMAP(JOY_LSHIFT_PADDLE) | BITMAP(JOY_RSHIFT_PADDLE) | BITMAP(JOY_START));
    inputHub::cycleCPWorkingMode_setBitmap(BITMAP(JOY_START) | BITMAP(JOY_LSHIFT_PADDLE));
    inputHub::cycleALTButtonsWorkingMode_setBitmap(BITMAP(JOY_START) | BITMAP(JOY_RSHIFT_PADDLE));
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    simWheelSetup();
    userSettings::begin();
    hidImplementation::begin(
        DEVICE_NAME,
        DEVICE_MANUFACTURER,
        false);
    inputs::start();
}

void loop()
{
    vTaskDelay(portMAX_DELAY);
}
