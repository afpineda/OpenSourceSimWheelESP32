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

#include "SimWheel.hpp"
#include "SimWheelUI.hpp"

//------------------------------------------------------------------
// Global customization
//------------------------------------------------------------------

/* -----------------------------------------------------------------
 >>>> [EN] DEVICE IDENTIFICATION
 >>>> [ES] IDENTIFICACIÓN DEL DISPOSITIVO
------------------------------------------------------------------ */

std::string DEVICE_NAME = "Open button box-2";
std::string DEVICE_MANUFACTURER = "Mamandurrio";

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------

void simWheelSetup()
{

    OutputGPIOCollection mtxSelectors = {
        GPIO_NUM_15,
        GPIO_NUM_2,
        GPIO_NUM_0,
        GPIO_NUM_4,
        GPIO_NUM_16,
        GPIO_NUM_17,
        GPIO_NUM_5};
    InputGPIOCollection mtxInputs = {
        GPIO_NUM_36,
        GPIO_NUM_39,
        GPIO_NUM_34,
        GPIO_NUM_35,
        GPIO_NUM_32,
        GPIO_NUM_33,
        GPIO_NUM_25,
        GPIO_NUM_26};
    ButtonMatrix mtx;
    populateButtonMatrix(mtx, mtxSelectors, mtxInputs, 0);
    inputs::addButtonMatrix(mtx);

    inputs::addRotaryEncoder(GPIO_NUM_27, GPIO_NUM_14, 56, 57);
    inputs::addRotaryEncoder(GPIO_NUM_12, GPIO_NUM_13, 58, 59);
    inputs::addRotaryEncoder(GPIO_NUM_18, GPIO_NUM_19, 60, 61);
    inputs::addRotaryEncoder(GPIO_NUM_22, GPIO_NUM_23, 62, 63);
}

void customFirmware()
{
    simWheelSetup();
    hid::configure(
        DEVICE_NAME,
        DEVICE_MANUFACTURER,
        false);
    firmware::run();
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