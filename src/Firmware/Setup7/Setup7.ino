/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-02-11
 * @brief Sim wheel setup #7
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
#define LCLUTCH 30
#define RCLUTCH 31

/* -----------------------------------------------------------------
 >>>> [EN] DEVICE IDENTIFICATION
 >>>> [ES] IDENTIFICACIÓN DEL DISPOSITIVO
------------------------------------------------------------------ */

// [EN] Set a name for this device between double quotes
// [ES] Indique un nombre para este dispositivo entre comillas

std::string DEVICE_NAME = "Open steering wheel-7";

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

// #define BATTERY_ENABLE_READ_GPIO GPIO_NUM_4

// [EN] Set an ADC-capable GPIO number for the "battREAD" pin.
// [ES] Indique el número de GPIO para el pin ADC de "battREAD".

#define BATTERY_READ_GPIO -1

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
        GPIO_NUM_34,
        GPIO_NUM_48,
        GPIO_NUM_18,
        srNumbers,
        sizeof(srNumbers) / sizeof(srNumbers[0]));
    inputs::addRotaryEncoder(GPIO_NUM_33, GPIO_NUM_39, ROT1_CW, ROT1_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_38, GPIO_NUM_37, ROT2_CW, ROT2_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_36, GPIO_NUM_35, ROT3_CW, ROT3_CCW);

    inputs::setAnalogClutchPaddles(GPIO_NUM_16, GPIO_NUM_17, LCLUTCH, RCLUTCH);
    inputHub::setClutchCalibrationButtons(ROT1_CW, ROT1_CCW);
    inputHub::cycleCPWorkingMode_setBitmap(BITMAP(JOY_START) | BITMAP(JOY_LB));
    inputHub::setCalibrationCommandBitmaps(
        BITMAP(JOY_START) | BITMAP(JOY_LB) | BITMAP(JOY_RB));
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
    clutchState::begin();
    inputs::begin();
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