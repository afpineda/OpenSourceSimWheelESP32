/**
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

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

bool configMenu::toggle()
{
    return false;
}

void configMenu::onInput(inputBitmap_t globalState, inputBitmap_t changes)
{
}

void uartServer::onReceive(char *text)
{
}

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------

void simWheelSetup()
{
    inputs::setButtonMatrix(
        mtxSelectors,
        sizeof(mtxSelectors) / sizeof(mtxSelectors[0]),
        mtxInputs,
        sizeof(mtxInputs) / sizeof(mtxInputs[0]));      // fistButtonNumber=0
    inputs::addRotaryEncoder(GPIO_NUM_26, GPIO_NUM_27); // fistButtonNumber=56
    inputs::addRotaryEncoder(GPIO_NUM_12, GPIO_NUM_13); // fistButtonNumber=58
    inputs::addRotaryEncoder(GPIO_NUM_18, GPIO_NUM_19); // fistButtonNumber=60
    inputs::addRotaryEncoder(GPIO_NUM_22, GPIO_NUM_23); // fistButtonNumber=62
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    inputs::begin();
    inputHub::begin();
    simWheelSetup();
    hidImplementation::begin(
        DEVICE_NAME,
        DEVICE_MANUFACTURER,
        false,
        false);

    inputs::start();
}

void loop()
{
    vTaskDelay(portMAX_DELAY);
}