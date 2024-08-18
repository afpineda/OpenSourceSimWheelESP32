/**
 * @file Setup2.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-05-01
 * @brief Sim wheel setup #2
 *
 * @copyright Licensed under the EUPL
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

static const gpio_num_array_t mtxSelectors = {
    GPIO_NUM_15,
    GPIO_NUM_2,
    GPIO_NUM_0,
    GPIO_NUM_4,
    GPIO_NUM_16,
    GPIO_NUM_17,
    GPIO_NUM_5};

static const gpio_num_array_t mtxInputs = {
    GPIO_NUM_36,
    GPIO_NUM_39,
    GPIO_NUM_34,
    GPIO_NUM_35,
    GPIO_NUM_32,
    GPIO_NUM_33,
    GPIO_NUM_25,
    GPIO_NUM_26};

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------

void simWheelSetup()
{
    auto &assignments = inputs::addButtonMatrix(mtxSelectors, mtxInputs);
    inputNumber_t inputNumber = 0;
    for (int inputIndex = 0; inputIndex < mtxInputs.size(); inputIndex++)
        for (int selectorIndex = 0; selectorIndex < mtxSelectors.size(); selectorIndex++)
            assignments.inputNumber(mtxSelectors[selectorIndex], mtxInputs[inputIndex], inputNumber++);

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