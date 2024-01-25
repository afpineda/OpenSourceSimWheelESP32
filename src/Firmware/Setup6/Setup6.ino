/**
 * @file Setup6.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-02-15
 * @brief Sim wheel setup #6
 *
 * @copyright Licensed under the EUPL
 *
 */

//#include <Arduino.h>
#include "SimWheel.h"

//------------------------------------------------------------------
// Global customization
//------------------------------------------------------------------

#define ALT1 16
#define ALT2 17
#define ROT1_SW 18
#define ROT2_SW 19
#define ROT3_SW 20
#define ROT4_SW 21
#define ROT1_CW 22
#define ROT1_CCW 23
#define ROT2_CW 24
#define ROT2_CCW 25
#define ROT3_CW 26
#define ROT3_CCW 27
#define ROT4_CW 28
#define ROT4_CCW 29
#define LCLUTCH 30
#define RCLUTCH 31

/* -----------------------------------------------------------------
 >>>> [EN] DEVICE IDENTIFICATION
 >>>> [ES] IDENTIFICACIÓN DEL DISPOSITIVO
------------------------------------------------------------------ */

// [EN] Set a name for this device between double quotes
// [ES] Indique un nombre para este dispositivo entre comillas

std::string DEVICE_NAME = "Open steering wheel-6";

// [EN] Set a manufacturer's name for this device between double quotes
// [ES] Indique un nombre para el fabricante de este dispositivo entre comillas

std::string DEVICE_MANUFACTURER = "Mamandurrio";

/* -----------------------------------------------------------------
 >>>> [EN] DEEP SLEEP MODE
 >>>> [ES] MODO DE SUEÑO PROFUNDO
------------------------------------------------------------------ */

// [EN] Wake up source: put a list of GPIO numbers between curly brackets.
//      If empty, only a RESET will wake up the system.
// [ES] Señales para despertar: indicar una lista de  numeros de GPIO entre llaves.
//      Si lo deja vacío, solamente un RESET despertará al sistema.

const gpio_num_t WAKEUP_PINS[] = {GPIO_NUM_32};

// [EN] Set to "true" or "false".
//      If "true", wake up happens when any given pin is set to high voltage.
//      If "false", wake up happens when all given pins are set to low voltage.
// [ES] Seleccione "true" o "false"
//      Con "true", se despierta con voltaje alto en cualquiera de los pines.
//      Con "false", se despierta con voltaje bajo en todos los pines.

#define WAKEUP_ANYorALL false

/* -----------------------------------------------------------------
 >>>> [EN] MULTIPLEXED BUTTONS
 >>>> [ES] BOTONES MULTIPLEXADOS
------------------------------------------------------------------ */

// [EN] Set all GPIO numbers for selector pins between curly brackets
// [ES] Indique los números de GPIO de todos los pines selectores entre las llaves
static const gpio_num_t mpxSelectors[] = {GPIO_NUM_16, GPIO_NUM_17, GPIO_NUM_5};

// [EN] Set all GPIO numbers for input pins between curly brackets
// [ES] Indique los números de GPIO de todos los pines de entrada entre las llaves
static const gpio_num_t mpxInputs[] = {GPIO_NUM_14, GPIO_NUM_22};

// [EN] Set all input numbers. The order of those numbers depends on the wiring
// [ES] Indique los números de entrada. Su orden depende del cableado.
static inputNumber_t mpxNumbers[] = {
    10, JOY_BACK, JOY_A, 14,
    ALT2, 11, 15, 13,
    JOY_Y, JOY_START, 9, JOY_B,
    ALT1, JOY_X, 12, 8};

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------

void simWheelSetup()
{
    inputs::addAnalogMultiplexer(
        mpxSelectors,
        sizeof(mpxSelectors) / sizeof(mpxSelectors[0]),
        mpxInputs,
        sizeof(mpxInputs) / sizeof(mpxInputs[0]),
        mpxNumbers);
    inputs::addRotaryEncoder(GPIO_NUM_32, GPIO_NUM_33, ROT1_CW, ROT1_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_25, GPIO_NUM_26, ROT2_CW, ROT2_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_27, GPIO_NUM_12, ROT3_CW, ROT3_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_13, GPIO_NUM_15, ROT4_CW, ROT4_CCW);
    inputs::addDigital(GPIO_NUM_2, ROT1_SW);
    inputs::addDigital(GPIO_NUM_0, ROT2_SW);
    inputs::addDigital(GPIO_NUM_4, ROT3_SW);
    inputs::addDigital(GPIO_NUM_18, ROT4_SW);
    inputs::addDigital(GPIO_NUM_23, JOY_LSHIFT_PADDLE);
    inputs::addDigital(GPIO_NUM_19, JOY_RSHIFT_PADDLE);
    inputs::setAnalogClutchPaddles(GPIO_NUM_36, GPIO_NUM_39);
    inputHub::setClutchInputNumbers(LCLUTCH, RCLUTCH);
    inputHub::setALTBitmap(BITMAP(ALT1) | BITMAP(ALT2));
    inputHub::setClutchCalibrationButtons(ROT1_CW, ROT1_CCW);
    inputHub::cycleCPWorkingMode_setBitmap(BITMAP(ROT1_SW) | BITMAP(JOY_LB));
    inputHub::cycleALTButtonsWorkingMode_setBitmap(BITMAP(ROT1_SW) | BITMAP(JOY_RB));
    inputHub::setCalibrationCommandBitmaps(
        BITMAP(ROT1_SW) |
        BITMAP(JOY_RB) |
        BITMAP(JOY_LB));
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    power::begin(
        WAKEUP_PINS,
        sizeof(WAKEUP_PINS) / sizeof(gpio_num_t),
        WAKEUP_ANYorALL);

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