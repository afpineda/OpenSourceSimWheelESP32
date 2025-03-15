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

#include "SimWheel.hpp"
#include "SimWheelUI.hpp"

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

// [EN] Set a GPIO number for "wake up".
//      Comment out if not required, or set an RTC-capable GPIO number for wake up.
// [ES] Indique el número de GPIO para la señal "despertar"
//      Comente la línea si no hay necesidad de entrar en sueño profundo, o bien,
//      indique un número de GPIO con capacidad RTC para despertar del sueño.

// #define WAKE_UP_PIN <here>

// [EN] Substitute <here> with a GPIO number or alias
// [ES] Sustituya <here> con un número de pin o su alias

/* -----------------------------------------------------------------
 >>>> [EN] POWER LATCH SUBSYSTEM
 >>>> [ES] SUBSISTEMA DE CERROJO DE ENERGÍA
------------------------------------------------------------------ */

// [EN] Set an output-capable GPIO number for the "POWER_LATCH" pin.
//      Comment out if there is no external power latch circuit.
// [ES] Indique el número de GPIO para la señal "POWER_LATCH"
//      Comente la línea si no hay circuito externo de power latch.

// #define POWER_LATCH <here>

// [EN] Substitute <here> with a GPIO number or alias
// [ES] Sustituya <here> con un número de pin o su alias

// [EN] Set a latch mode
// [ES] Ajuste un mode de activación

#define LATCH_MODE PowerLatchMode::POWER_OPEN_DRAIN

// [EN] Set a delay (in milliseconds) to wait for the latch circuit
//      to do its magic (optional)
// [ES] Ajuste un retardo (en milisegundos) a esperar para
//      que el circuito haga su magia

#define LATCH_POWEROFF_DELAY 3000

/* -----------------------------------------------------------------
 >>>> [EN] BATTERY MONITOR SUBSYSTEM
 >>>> [ES] SUBSISTEMA DE MONITORIZACIÓN DE BATERÍA
------------------------------------------------------------------ */

// [EN] Comment out the following line if a "fuel gauge" is NOT in place
// [ES] Comentar la siguiente linea si NO existe un "fuel gauge"

// #define ENABLE_FUEL_GAUGE

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

// [EN] Uncomment the following lines to set a custom VID/PID
// [ES] Descomente la siguiente linea para ajustar el VID/PID a medida

// #define BLE_CUSTOM_VID <here>
// #define BLE_CUSTOM_PID <here>

// [EN] Substitute <here> with a non-zero 16-bits number as
//      a custom vendor/product ID
// [ES] Sustituya <here> con un número de 16 bits distinto de cero
//      como identificador de fabricante/producto a medida

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
    inputs::addButton(GPIO_NUM_34, 33);
    inputs::addButton(GPIO_NUM_33, 34);
    inputs::addButton(GPIO_NUM_27, 35);

    inputHub::clutch::inputs(33, 34);
    inputHub::clutch::bitePointInputs(31, 32); // Rotary 4
    inputHub::dpad::inputs(25, 26, 27, 28);
    inputHub::altButtons::inputs({35});
}

//------------------------------------------------------------------

void customFirmware()
{

#ifdef WAKE_UP_PIN
    power::configureWakeUp(WAKE_UP_PIN);
#endif

#ifdef POWER_LATCH
    power::configurePowerLatch(
        POWER_LATCH,
        LATCH_MODE,
        LATCH_POWEROFF_DELAY);
#endif

    simWheelSetup();
    hid::configure(
        DEVICE_NAME,
        DEVICE_MANUFACTURER,
        true
#if defined(BLE_CUSTOM_VID)
        ,
        BLE_CUSTOM_VID
#if defined(BLE_CUSTOM_PID)
        ,
        BLE_CUSTOM_PID
#endif
#endif
    );

#if defined(ENABLE_FUEL_GAUGE)
    batteryMonitor::configure();
#elif defined(ENABLE_BATTERY_MONITOR)
    batteryMonitor::configure(
        BATTERY_READ_GPIO,
        BATTERY_ENABLE_READ_GPIO);
#endif
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