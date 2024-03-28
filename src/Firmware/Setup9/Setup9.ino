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
    inputs::addDigital(GPIO_NUM_33, JOY_A);
    inputs::addDigital(GPIO_NUM_34, NAV_LEFT);
    inputs::addDigital(GPIO_NUM_17, NAV_RIGHT);
    inputs::addDigital(GPIO_NUM_38, NAV_UP);
    inputs::addDigital(GPIO_NUM_18, NAV_DOWN);
    inputs::setAnalogClutchPaddles(GPIO_NUM_14, GPIO_NUM_15);
    inputs::addMCP23017Digital(7, false)
        .inputNumber(MCP23017_pin_t::GPA0, JOY_LSHIFT_PADDLE)
        .inputNumber(MCP23017_pin_t::GPA1, JOY_RSHIFT_PADDLE)
        .inputNumber(MCP23017_pin_t::GPA2, ALT1)
        .inputNumber(MCP23017_pin_t::GPA3, ALT2)
        .inputNumber(MCP23017_pin_t::GPA4, JOY_LTHUMBSTICK_CLICK)
        .inputNumber(MCP23017_pin_t::GPA5, JOY_RTHUMBSTICK_CLICK)
        .inputNumber(MCP23017_pin_t::GPA6, 10)
        .inputNumber(MCP23017_pin_t::GPA7, 11)
        .inputNumber(MCP23017_pin_t::GPB0, 12)
        .inputNumber(MCP23017_pin_t::GPB1, 13)
        .inputNumber(MCP23017_pin_t::GPB2, 14)
        .inputNumber(MCP23017_pin_t::GPB3, 15)
        .inputNumber(MCP23017_pin_t::GPB4, 16)
        .inputNumber(MCP23017_pin_t::GPB5, 17)
        .inputNumber(MCP23017_pin_t::GPB6, 18)
        .inputNumber(MCP23017_pin_t::GPB7, 19);
    inputs::addMCP23017Digital(0, false)
        .inputNumber(MCP23017_pin_t::GPA0, 20)
        .inputNumber(MCP23017_pin_t::GPA1, 21)
        .inputNumber(MCP23017_pin_t::GPA2, 22)
        .inputNumber(MCP23017_pin_t::GPA3, 23)
        .inputNumber(MCP23017_pin_t::GPA4, 24)
        .inputNumber(MCP23017_pin_t::GPA5, 25)
        .inputNumber(MCP23017_pin_t::GPA6, 26)
        .inputNumber(MCP23017_pin_t::GPA7, 27)
        .inputNumber(MCP23017_pin_t::GPB0, 28)
        .inputNumber(MCP23017_pin_t::GPB1, 29)
        .inputNumber(MCP23017_pin_t::GPB2, 30)
        .inputNumber(MCP23017_pin_t::GPB3, JOY_Y)
        .inputNumber(MCP23017_pin_t::GPB4, JOY_X)
        .inputNumber(MCP23017_pin_t::GPB5, JOY_B)
        .inputNumber(MCP23017_pin_t::GPB6, JOY_BACK)
        .inputNumber(MCP23017_pin_t::GPB7, JOY_START);

    inputHub::setClutchInputNumbers(CLUTCH1, CLUTCH2);
    inputHub::setDPADControls(NAV_UP, NAV_DOWN, NAV_LEFT, NAV_RIGHT);
    inputHub::setALTInputNumbers({(ALT1), (ALT2)});
    inputHub::setClutchCalibrationInputNumbers(ROT1_CW, ROT1_CCW); // Rotary 1
    inputHub::cmdRecalibrateAnalogAxis_setInputNumbers({(JOY_LSHIFT_PADDLE), (JOY_RSHIFT_PADDLE), (JOY_START)});
    inputHub::cycleCPWorkingMode_setInputNumbers({(JOY_START), (JOY_LSHIFT_PADDLE)});
    inputHub::cycleALTButtonsWorkingMode_setInputNumbers({(JOY_START), (JOY_RSHIFT_PADDLE)});
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
