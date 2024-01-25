/**
 * @file Setup5.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-02-11
 * @brief Sim wheel setup #5
 *
 * @copyright Licensed under the EUPL
 *
 */

//#include <Arduino.h>
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
#define LCLUTCH 30
#define RCLUTCH 31

/* -----------------------------------------------------------------
 >>>> [EN] DEVICE IDENTIFICATION
 >>>> [ES] IDENTIFICACIÓN DEL DISPOSITIVO
------------------------------------------------------------------ */

// [EN] Set a name for this device between double quotes
// [ES] Indique un nombre para este dispositivo entre comillas

std::string DEVICE_NAME = "Open steering wheel-5";

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

const gpio_num_t WAKEUP_PINS[] = {GPIO_NUM_33};

// [EN] Set to "true" or "false".
//      If "true", wake up happens when any given pin is set to high voltage.
//      If "false", wake up happens when all given pins are set to low voltage.
// [ES] Seleccione "true" o "false"
//      Con "true", se despierta con voltaje alto en cualquiera de los pines.
//      Con "false", se despierta con voltaje bajo en todos los pines.

#define WAKEUP_ANYorALL false

/* -----------------------------------------------------------------
 >>>> [EN] BATTERY MONITOR SUBSYSTEM
 >>>> [ES] SUBSISTEMA DE MONITORIZACIÓN DE BATERÍA
------------------------------------------------------------------ */

// [EN] Set an output-capable GPIO number for the "battEN" pin.
// [ES] Indique el número de GPIO para el pin "battEN".

//#define BATTERY_ENABLE_READ_GPIO GPIO_NUM_4

// [EN] Set an ADC-capable GPIO number for the "battREAD" pin.
// [ES] Indique el número de GPIO para el pin ADC de "battREAD".

#define BATTERY_READ_GPIO -1

/* -----------------------------------------------------------------
 >>>> [EN] MULTIPLEXED BUTTONS
 >>>> [ES] BOTONES MULTIPLEXADOS
------------------------------------------------------------------ */

// [EN] Set all GPIO numbers for selector pins between curly brackets
// [ES] Indique los números de GPIO de todos los pines selectores entre las llaves
static const gpio_num_t mpxSelectors[] = {GPIO_NUM_48, GPIO_NUM_18, GPIO_NUM_17};

// [EN] Set all GPIO numbers for input pins between curly brackets
// [ES] Indique los números de GPIO de todos los pines de entrada entre las llaves
static const gpio_num_t mpxInputs[] = {GPIO_NUM_16, GPIO_NUM_34};

// [EN] Set all input numbers. The order of those numbers depends on the wiring
// [ES] Indique los números de entrada. Su orden depende del cableado.
static inputNumber_t mpxNumbers[] = {
    JOY_BACK, JOY_LB, LCLUTCH, JOY_A,
    RCLUTCH, JOY_B, JOY_RB, JOY_START,
    10, 8, JOY_X, 12,
    JOY_Y, 13, 9, 11};

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
    inputs::addRotaryEncoder(GPIO_NUM_33, GPIO_NUM_39, ROT1_CW, ROT1_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_38, GPIO_NUM_37, ROT2_CW, ROT2_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_36, GPIO_NUM_35, ROT3_CW, ROT3_CCW);
    inputHub::setClutchInputNumbers(LCLUTCH, RCLUTCH);
    inputHub::setClutchCalibrationButtons(ROT1_CW, ROT1_CCW);
    inputHub::cycleCPWorkingMode_setBitmap(BITMAP(JOY_START) | BITMAP(JOY_LB));
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

#ifdef BATTERY_ENABLE_READ_GPIO
    batteryCalibration::begin();
    power::startBatteryMonitor(
        (gpio_num_t)BATTERY_ENABLE_READ_GPIO,
        (gpio_num_t)BATTERY_READ_GPIO);
    hidImplementation::begin(
        DEVICE_NAME,
        DEVICE_MANUFACTURER,
        true);
#else
    hidImplementation::begin(
        DEVICE_NAME,
        DEVICE_MANUFACTURER,
        false);
#endif

    inputs::start();
}

void loop()
{
    vTaskDelay(portMAX_DELAY);
}