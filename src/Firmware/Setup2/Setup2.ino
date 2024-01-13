/**
 * @file Setup2.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-05-01
 * @brief Sim wheel setup #2
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "SimWheel.h"

//------------------------------------------------------------------
// Global customization
//------------------------------------------------------------------

/* -----------------------------------------------------------------
 >>>> [EN] DEVICE IDENTIFICATION
 >>>> [ES] IDENTIFICACIÓN DEL DISPOSITIVO
------------------------------------------------------------------ */

std::string DEVICE_NAME = "Open button box-2";
std::string DEVICE_MANUFACTURER = "Mamandurrio";

/* -----------------------------------------------------------------
 >>>> [EN] BUTTON MATRIX
 >>>> [ES] MATRIZ DE BOTONES
------------------------------------------------------------------ */

static const gpio_num_t mtxSelectors[] = {
    GPIO_NUM_15,
    GPIO_NUM_2,
    GPIO_NUM_0,
    GPIO_NUM_4,
    GPIO_NUM_16,
    GPIO_NUM_17,
    GPIO_NUM_5};

static const gpio_num_t mtxInputs[] = {
    GPIO_NUM_36,
    GPIO_NUM_39,
    GPIO_NUM_34,
    GPIO_NUM_35,
    GPIO_NUM_32,
    GPIO_NUM_33,
    GPIO_NUM_25,
    GPIO_NUM_26};

static inputNumber_t mtxNumbers[] = {
    0, 1, 2, 3, 4, 5, 6,
    7, 8, 9, 10, 11, 12, 13,
    14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27,
    28, 29, 30, 31, 32, 33, 34,
    35, 36, 37, 38, 39, 40, 41,
    42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 52, 53, 54, 55};

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

void batteryCalibration::restartAutoCalibration()
{

}

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
    inputs::addRotaryEncoder(GPIO_NUM_26, GPIO_NUM_27, 56, 57);
    inputs::addRotaryEncoder(GPIO_NUM_12, GPIO_NUM_13, 58, 59);
    inputs::addRotaryEncoder(GPIO_NUM_18, GPIO_NUM_19, 60, 61);
    inputs::addRotaryEncoder(GPIO_NUM_22, GPIO_NUM_23, 62, 63);
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
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