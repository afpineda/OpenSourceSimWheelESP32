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

#include <Arduino.h>
#include "SimWheel.h"

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

/* -----------------------------------------------------------------
 >>>> [EN] BUTTON MATRIX
 >>>> [ES] MATRIZ DE BOTONES
------------------------------------------------------------------ */

static const gpio_num_array_t mtxSelectors = {
    GPIO_NUM_4,
    GPIO_NUM_16,
    GPIO_NUM_17,
    GPIO_NUM_1,
    GPIO_NUM_23};

static const gpio_num_array_t mtxInputs = {
    GPIO_NUM_15,
    GPIO_NUM_5,
    GPIO_NUM_21,
    GPIO_NUM_22,
    GPIO_NUM_13};

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------

void simWheelSetup()
{
    inputs::addButtonMatrix(mtxSelectors, mtxInputs)
        .inputNumber(GPIO_NUM_4, GPIO_NUM_15, JOY_A)
        .inputNumber(GPIO_NUM_16, GPIO_NUM_15, JOY_B)
        .inputNumber(GPIO_NUM_17, GPIO_NUM_15, JOY_X)
        .inputNumber(GPIO_NUM_1, GPIO_NUM_15, JOY_Y)
        .inputNumber(GPIO_NUM_23, GPIO_NUM_15, JOY_BACK)
        .inputNumber(GPIO_NUM_4, GPIO_NUM_5, JOY_START)
        .inputNumber(GPIO_NUM_16, GPIO_NUM_5, 8)
        .inputNumber(GPIO_NUM_17, GPIO_NUM_5, 9)
        .inputNumber(GPIO_NUM_1, GPIO_NUM_5, 10)
        .inputNumber(GPIO_NUM_23, GPIO_NUM_5, 11)
        .inputNumber(GPIO_NUM_4, GPIO_NUM_21, 12)
        .inputNumber(GPIO_NUM_16, GPIO_NUM_21, 13)
        .inputNumber(GPIO_NUM_17, GPIO_NUM_21, 14)
        .inputNumber(GPIO_NUM_1, GPIO_NUM_21, 15)
        .inputNumber(GPIO_NUM_23, GPIO_NUM_21, 16)
        .inputNumber(GPIO_NUM_4, GPIO_NUM_22, 17)
        .inputNumber(GPIO_NUM_16, GPIO_NUM_22, IN_UP)
        .inputNumber(GPIO_NUM_17, GPIO_NUM_22, IN_DOWN)
        .inputNumber(GPIO_NUM_1, GPIO_NUM_22, IN_LEFT)
        .inputNumber(GPIO_NUM_23, GPIO_NUM_22, IN_RIGHT)
        .inputNumber(GPIO_NUM_4, GPIO_NUM_13, IN_ALT2)
        .inputNumber(GPIO_NUM_16, GPIO_NUM_13, IN_ALT1)
        .inputNumber(GPIO_NUM_17, GPIO_NUM_13, JOY_RTHUMBSTICK_CLICK)
        .inputNumber(GPIO_NUM_1, GPIO_NUM_13, JOY_LTHUMBSTICK_CLICK)
        .inputNumber(GPIO_NUM_23, GPIO_NUM_13, 63);

    inputs::setAnalogClutchPaddles(GPIO_NUM_36, GPIO_NUM_39); // 18 and 19
    inputs::addRotaryEncoder(GPIO_NUM_34, GPIO_NUM_35, 40, 41);
    inputs::addRotaryEncoder(GPIO_NUM_32, GPIO_NUM_33, 42, 43);
    inputs::addRotaryEncoder(GPIO_NUM_25, GPIO_NUM_26, 44, 45);
    inputs::addRotaryEncoder(GPIO_NUM_27, GPIO_NUM_14, 46, 47);
    inputs::addRotaryEncoder(GPIO_NUM_19, GPIO_NUM_18, 48, 49, true); // ALPS

    inputHub::setDPADControls(IN_UP, IN_DOWN, IN_LEFT, IN_RIGHT);
    inputHub::setALTInputNumbers({IN_ALT1,IN_ALT2});
    inputHub::setClutchInputNumbers(18, 19);
    inputHub::setClutchCalibrationInputNumbers(40, 41); // Rotary 1
    inputHub::cycleCPWorkingMode_setInputNumbers({JOY_START,JOY_LB});
    inputHub::cycleALTButtonsWorkingMode_setInputNumbers({JOY_START,JOY_RB});
    inputHub::cmdRecalibrateAnalogAxis_setInputNumbers({JOY_START,JOY_LB,JOY_RB});
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    userSettings::begin();
    simWheelSetup();
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