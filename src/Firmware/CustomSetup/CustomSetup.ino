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

// [EN] Set an output-capable GPIO number for the "wake up" pin.
//      Comment out if not required, or set an RTC-capable GPIO number for wake up.
// [ES] Indique el número de GPIO para la señal "despertar"
//      Comente la línea si no hay necesidad de entrar en sueño profundo, o bien,
//      indique un número de GPIO con capacidad RTC para despertar del sueño.

// #define WAKE_UP_PIN

/* -----------------------------------------------------------------
 >>>> [EN] POWER LATCH SUBSYSTEM
 >>>> [ES] SUBSISTEMA DE CERROJO DE ENERGÍA
------------------------------------------------------------------ */

// [EN] Set an output-capable GPIO number for the "POWER_LATCH" pin.
//      Comment out if there is no external power latch circuit.
// [ES] Indique el número de GPIO para la señal "POWER_LATCH"
//      Comente la línea si no hay circuito externo de power latch.

// #define POWER_LATCH

#ifdef POWER_LATCH
// [EN] Set an the activation mode for the "POWER_LATCH" pin.
//      Ignore if there is no external power latch circuit
//      POWER_OPEN_DRAIN --> Power on when open drain, power off when low voltage
//      POWER_OFF_HIGH --> Power on when low voltage, power off when high voltage
//      POWER_OFF_LOW --> Power on when high voltage, power off when low voltage
// [ES] Elija el modo de activación del pin "POWER_LATCH".
//      Ignóralo si no hay circuito externo de power latch
//      POWER_OPEN_DRAIN --> Encendido mediante circuito abierto, apagado a bajo voltaje
//      POWER_OFF_HIGH --> Encendido a bajo voltaje, apagado a alto voltaje
//      POWER_OFF_LOW --> Encendido a alto voltaje, apagado a bajo voltaje

const powerLatchMode_t LATCH_MODE = POWER_OPEN_DRAIN;

// [EN] Set a time delay, in milliseconds, to wait for the latch circuit to power
//      the system off. Ignore if there is no external power latch circuit
// [ES] Indicar un retardo, en milisegundos, a esperar para que el circuito apague
//      el sistema. Ignóralo si no hay circuito externo de power latch.

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
 >>>> [EN] DEVICE IDENTIFICATION
 >>>> [ES] IDENTIFICATION DEL DISPOSITIVO
------------------------------------------------------------------ */

// [EN] Uncomment the following line to set a custom PID
// [ES] Descomente la siguiente linea para ajustar el PID a medida

// #define BLE_CUSTOM_PID <here>

// [EN] Substitute <here> with a non-zero 16-bits number as
//      a custom product ID
// [ES] Sustituya <here> con un número de 16 bits distinto de cero
//      como identificador de producto a medida

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------

void simWheelSetup()
{
    // [EN] Example code. Delete and fill with your own code.
    // [ES] Código de ejemplo. Bórrelo y ponga el suyo.

    inputs::addRotaryEncoder(GPIO_NUM_36, GPIO_NUM_39, 25, 26);
    inputs::addRotaryEncoder(GPIO_NUM_35, GPIO_NUM_32, 27, 28);
    inputs::addRotaryEncoder(GPIO_NUM_25, GPIO_NUM_26, 29, 30);
    inputs::addRotaryEncoder(GPIO_NUM_14, GPIO_NUM_18, 31, 32);
    inputs::addDigital(GPIO_NUM_34, 33);
    inputs::addDigital(GPIO_NUM_33, 34);
    inputs::addDigital(GPIO_NUM_27, 35);

    inputHub::setClutchInputNumbers(33, 34);
    inputHub::setClutchCalibrationInputNumbers(31, 32); // Rotary 4
    inputHub::setDPADControls(25, 26, 27, 28);
    inputHub::setALTInputNumbers({35});
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);

#ifdef WAKE_UP_PIN
    power::begin((gpio_num_t)WAKE_UP_PIN);
#endif

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
        DEVICE_MANUFACTURER
#if BLE_CUSTOM_PID != 0
        ,
        BLE_CUSTOM_PID
#endif
    );

#ifdef ENABLE_BATTERY_MONITOR
    batteryCalibration::begin();
    batteryMonitor::begin(
        (gpio_num_t)BATTERY_ENABLE_READ_GPIO,
        (gpio_num_t)BATTERY_READ_GPIO);
#endif

    inputs::start();
}

void loop()
{
    vTaskDelay(portMAX_DELAY);
}