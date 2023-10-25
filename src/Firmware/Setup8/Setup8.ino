/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-10-25
 * @brief Sim wheel setup #8
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "SimWheel.h"

//------------------------------------------------------------------
// Global customization
//------------------------------------------------------------------

#define ROT1_CW 40
#define ROT1_CCW 41
#define ROT2_CW 42
#define ROT2_CCW 43
#define ROT3_CW 44
#define ROT3_CCW 45
#define ROT4_CW 46
#define ROT4_CCW 47

/* -----------------------------------------------------------------
 >>>> [EN] DEVICE IDENTIFICATION
 >>>> [ES] IDENTIFICACIÓN DEL DISPOSITIVO
------------------------------------------------------------------ */

// [EN] Set a name for this device between double quotes
// [ES] Indique un nombre para este dispositivo entre comillas

std::string DEVICE_NAME = "ButtonBox-8";

// [EN] Set a manufacturer's name for this device between double quotes
// [ES] Indique un nombre para el fabricante de este dispositivo entre comillas

std::string DEVICE_MANUFACTURER = "Mamandurrio";

/* -----------------------------------------------------------------
 >>>> [EN] SHIFT REGISTERS
 >>>> [ES] REGISTROS DE DESPLAZAMIENTO
------------------------------------------------------------------ */

// [EN] Set all input numbers. The order of those numbers depends on the wiring
// [ES] Indique los números de entrada. Su orden depende del cableado.

static inputNumber_t srNumbers[] = {
    JOY_LB, JOY_A, 8, JOY_RB, 16, 15, 10, 9,
    JOY_B, JOY_X, JOY_Y, JOY_BACK, 14, 13, 12, 11,
    19, 20, 21, 22, JOY_START, 24, 18, 17,
    23 };

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------

void simWheelSetup()
{
    inputs::addShiftRegisters(
        GPIO_NUM_39,
        GPIO_NUM_33,
        GPIO_NUM_34,
        srNumbers,
        sizeof(srNumbers) / sizeof(srNumbers[0]));
    inputs::addRotaryEncoder(GPIO_NUM_38, GPIO_NUM_37, ROT1_CW, ROT1_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_36, GPIO_NUM_35, ROT2_CW, ROT2_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_48, GPIO_NUM_18, ROT3_CW, ROT3_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_17, GPIO_NUM_16, ROT4_CW, ROT4_CCW);
    
    // LilyGo T-QT built-in push buttons
    inputs::addDigital(GPIO_NUM_0,60);
    inputs::addDigital(GPIO_NUM_47,61);
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