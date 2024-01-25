/**
 * @file CustomSetup.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-01
 * @brief Custom setup. Please, read
 *        [How to customize](../../../doc/hardware/CustomizeHowto_en.md)
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

// [EN] Set a name for this device between double quotes
// [ES] Indique un nombre para este dispositivo entre comillas

std::string DEVICE_NAME = "Custom steering wheel";

// [EN] Set a manufacturer's name for this device between double quotes
// [ES] Indique un nombre para el fabricante de este dispositivo entre comillas

std::string DEVICE_MANUFACTURER = "Me";

/* -----------------------------------------------------------------
 >>>> [EN] DEEP SLEEP MODE
 >>>> [ES] MODO DE SUEÑO PROFUNDO
------------------------------------------------------------------ */

// [EN] Wake up source: put a list of GPIO numbers between curly brackets.
//      If empty, only a RESET will wake up the system.
// [ES] Señales para despertar: indicar una lista de  numeros de GPIO entre llaves.
//      Si lo deja vacío, solamente un RESET despertará al sistema.

const gpio_num_t WAKEUP_PINS[] = {};

// [EN] Set to "true" or "false".
//      If "true", wake up happens when any given pin are set to high voltage.
//      If "false", wake up happens when all given pins are set to low voltage.
// [ES] Seleccione "true" o "false"
//      Con "true", se despierta con voltaje alto en cualquiera de los pines.
//      Con "false", se despierta con voltaje bajo en todos los pines.

#define WAKEUP_ANYorALL false

/* -----------------------------------------------------------------
 >>>> [EN] POWER LATCH SUBSYSTEM
 >>>> [ES] SUBSISTEMA DE CERROJO DE ENERGÍA
------------------------------------------------------------------ */

// [EN] Set an output-capable GPIO number for the "POWER_LATCH" pin.
//      Comment out if there is no extenal power latch circuit.
// [ES] Indique el número de GPIO para la señal "POWER_LATCH"
//      Comente la línea si no hay circuito externo de power latch.

//#define POWER_LATCH GPIO_NUM_0

#ifdef POWER_LATCH
// [EN] Set an the activation mode for the "POWER_LATCH" pin.
//      Ignore if there is no external power latch circuit
//      POWER_OPEN_DRAIN --> Power on when open drain, power off when low voltage
//      POWER_OFF_HIGH --> Power on when low voltage, power off when high voltage
//      POWER_OFF_LOW --> Power on when high voltage, power off when low voltage
// [ES] Elija el modo de activación del pin "POWER_LATCH".
//      Ignórelo si no hay circuito externo de power latch
//      POWER_OPEN_DRAIN --> Encendido mediante circuito abierto, apagado a bajo voltaje
//      POWER_OFF_HIGH --> Encencido a bajo voltaje, apagado a alto voltaje
//      POWER_OFF_LOW --> Encencido a alto voltaje, apagado a bajo voltaje

const powerLatchMode_t LATCH_MODE = POWER_OPEN_DRAIN;

// [EN] Set a time delay, in milliseconds, to wait for the latch circuit to power
//      the system off. Ignore if there is no external power latch circuit
// [ES] Indicar un retardo, en milisegundos, a esperar para que el circuito apague
//      el sistema. Ignórelo si no hay circuito externo de power latch.

#define LATCH_POWEROFF_DELAY 3000

#endif // POWER_LATCH

/* -----------------------------------------------------------------
 >>>> [EN] BATTERY MONITOR SUBSYSTEM
 >>>> [ES] SUBSISTEMA DE MONITORIZACIÓN DE BATERÍA
------------------------------------------------------------------ */

// [EN] Comment out the following line if the battery monitor subsystem is NOT in place
// [ES] Comentar la siguiente linea si NO hay subsistema de monitorización de batería
#define ENABLE_BATTERY_MONITOR

#ifdef ENABLE_BATTERY_MONITOR
// [EN] Set an output-capable GPIO number for the "battEN" pin.
// [ES] Indique el número de GPIO para el pin "battEN".

#define BATTERY_ENABLE_READ_GPIO GPIO_NUM_1

// [EN] Set an ADC-capable GPIO number for the "battREAD" pin.
// [ES] Indique el número de GPIO para el pin ADC de "battREAD".

#define BATTERY_READ_GPIO GPIO_NUM_2

#endif // ENABLE_BATTERY_MONITOR

/* -----------------------------------------------------------------
 >>>> [EN] BUTTON MATRIX
 >>>> [ES] MATRIZ DE BOTONES
------------------------------------------------------------------ */

// [EN] Set all GPIO numbers for selector pins between curly brackets
// [ES] Indique los números de GPIO de todos los pines selectores entre las llaves
static const gpio_num_t mtxSelectors[] = {GPIO_NUM_4, GPIO_NUM_16, GPIO_NUM_17, GPIO_NUM_5, GPIO_NUM_12};

// [EN] Set all GPIO numbers for input pins between curly brackets
// [ES] Indique los números de GPIO de todos los pines de entrada entre las llaves
static const gpio_num_t mtxInputs[] = {GPIO_NUM_15, GPIO_NUM_19, GPIO_NUM_3, GPIO_NUM_23};

// [EN] Set all input numbers. The order of those numbers depends on the wiring
// [ES] Indique los números de entrada. Su orden depende del cableado.
static inputNumber_t mtxNumbers[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------

void simWheelSetup()
{
    // [EN] Example code. Fill with your own code.
    // [ES] Código de ejemplo. Ponga el suyo.

    inputs::addButtonMatrix(
        mtxSelectors,
        sizeof(mtxSelectors) / sizeof(mtxSelectors[0]),
        mtxInputs,
        sizeof(mtxInputs) / sizeof(mtxInputs[0]),
        mtxNumbers);
    inputs::addRotaryEncoder(GPIO_NUM_36, GPIO_NUM_39,25,26);
    inputs::addRotaryEncoder(GPIO_NUM_35, GPIO_NUM_32,27,28);
    inputs::addRotaryEncoder(GPIO_NUM_25, GPIO_NUM_26,29,30);
    inputs::addRotaryEncoder(GPIO_NUM_14, GPIO_NUM_18,31,32);
    inputs::addDigital(GPIO_NUM_34, true, false,33);
    inputs::addDigital(GPIO_NUM_33, true, false,34);
    inputs::addDigital(GPIO_NUM_27, true, false,35);

    inputHub::setClutchInputNumbers(4, 12);
    inputHub::setDPADControls(19, 15, 11, 7);
    inputHub::setALTBitmap(BITMAP(0) | BITMAP(8));
    inputHub::setClutchCalibrationButtons(25, 26); // Rotary 1
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

#ifdef POWER_LATCH
    power::setPowerLatch(
        (gpio_num_t)POWER_LATCH,
        LATCH_MODE,
        LATCH_POWEROFF_DELAY);
#endif

    userSettings::begin();
    simWheelSetup();
    hidImplementation::begin(
        DEVICE_NAME,
        DEVICE_MANUFACTURER);

#ifdef ENABLE_BATTERY_MONITOR
    batteryCalibration::begin();
    power::startBatteryMonitor(
        (gpio_num_t)BATTERY_ENABLE_READ_GPIO,
        (gpio_num_t)BATTERY_READ_GPIO);
#endif

    inputs::start();
}

void loop()
{
    vTaskDelay(portMAX_DELAY);
}