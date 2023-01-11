/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-01
 * @brief Sim wheel setup #1
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
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

static const gpio_num_t mtxSelectors[] = {
    GPIO_NUM_4,
    GPIO_NUM_16,
    GPIO_NUM_17,
    GPIO_NUM_1,
    GPIO_NUM_23};

static const gpio_num_t mtxInputs[] = {
    GPIO_NUM_15,
    GPIO_NUM_5,
    GPIO_NUM_21,
    GPIO_NUM_22,
    GPIO_NUM_13};

static inputNumber_t mtxNumbers[] = {
    JOY_A, JOY_B, JOY_X, JOY_Y, JOY_BACK,
    JOY_START, 8, 9, 10, 11,
    12, 13, 14, 15, 16,
    17, IN_UP, IN_DOWN, IN_LEFT, IN_RIGHT,
    IN_ALT2, IN_ALT1, JOY_RTHUMBSTICK_CLICK, JOY_LTHUMBSTICK_CLICK};

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------

void simWheelSetup()
{
    inputs::addButtonMatrix(
        mtxSelectors,
        sizeof(mtxSelectors) / sizeof(mtxSelectors[0]),
        mtxInputs,
        sizeof(mtxInputs) / sizeof(mtxInputs[0]),
        mtxNumbers);
    inputs::setAnalogClutchPaddles(GPIO_NUM_36, GPIO_NUM_39, 18, 19);
    inputs::addRotaryEncoder(GPIO_NUM_34, GPIO_NUM_35, 40, 41);
    inputs::addRotaryEncoder(GPIO_NUM_32, GPIO_NUM_33, 42, 43);
    inputs::addRotaryEncoder(GPIO_NUM_25, GPIO_NUM_26, 44, 45);
    inputs::addRotaryEncoder(GPIO_NUM_27, GPIO_NUM_14, 46, 47);
    inputs::addRotaryEncoder(GPIO_NUM_19, GPIO_NUM_18, 48, 49, true); // ALPS

    inputHub::setDPADControls(IN_UP, IN_DOWN, IN_LEFT, IN_RIGHT);
    inputHub::setALTBitmap(BITMAP(IN_ALT1) | BITMAP(IN_ALT2));
    inputHub::setClutchCalibrationButtons(40, 41); // Rotary 1
    inputHub::setCycleClutchFunctionBitmap(BITMAP(JOY_START) | BITMAP(JOY_LB));
    inputHub::setCycleALTFunctionBitmap(BITMAP(JOY_START) | BITMAP(JOY_RB));
    inputHub::setCalibrationCommandBitmaps(BITMAP(JOY_START) | BITMAP(JOY_LB) | BITMAP(JOY_RB), 0);
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    power::begin(
        nullptr,
        0,
        false);
    clutchState::begin();
    inputs::begin();
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