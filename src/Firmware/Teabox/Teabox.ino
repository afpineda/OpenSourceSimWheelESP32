/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-02-15
 * @brief Sim wheel setup "teabox"
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "SimWheel.h"

//------------------------------------------------------------------
// Global customization
//------------------------------------------------------------------

#define ROT1_SW JOY_A
#define BTN1 JOY_B
#define BTN2 JOY_X
#define BTN3 JOY_Y
#define ROT1_CW 24
#define ROT1_CCW 25
#define BTN_UP 26
#define BTN_DOWN 27

/* -----------------------------------------------------------------
 >>>> [EN] DEVICE IDENTIFICATION
 >>>> [ES] IDENTIFICACIÓN DEL DISPOSITIVO
------------------------------------------------------------------ */

// [EN] Set a name for this device between double quotes
// [ES] Indique un nombre para este dispositivo entre comillas

std::string DEVICE_NAME = "Teabox";

// [EN] Set a manufacturer's name for this device between double quotes
// [ES] Indique un nombre para el fabricante de este dispositivo entre comillas

std::string DEVICE_MANUFACTURER = "Mamandurrio";

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------

void simWheelSetup()
{
    inputs::addRotaryEncoder(GPIO_NUM_12, GPIO_NUM_14, ROT1_CW, ROT1_CCW);
    inputs::addDigital(GPIO_NUM_27, ROT1_SW);
    inputs::addDigital(GPIO_NUM_26, BTN_UP);
    inputs::addDigital(GPIO_NUM_25, BTN_DOWN);
    inputs::addDigital(GPIO_NUM_13, BTN1);
    inputs::addDigital(GPIO_NUM_15, BTN2);
    inputs::addDigital(GPIO_NUM_2, BTN3);
    inputHub::setDPADControls(BTN_UP,BTN_DOWN,ROT1_CCW,ROT1_CW);
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);

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