/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-01
 * @brief Custom setup. Please, read
 *        [How to customize](../../../doc/hardware/CustomizeHowto_en.md)
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
 >>>> OLED
------------------------------------------------------------------ */

// [EN] Set the display type to one of these.
//      Comment out if there is no OLED
// [ES] Indique el controlador de pantalla entre uno de estos
//      Comente la línea si no hay OLED
//   SSOLED_128x128
//   SSOLED_128x32,
//   SSOLED_128x64,
//   SSOLED_132x64,
//   SSOLED_64x32,
//   SSOLED_96x16,
//   SSOLED_72x40

#define OLED_TYPE SSOLED_132x64

#ifdef OLED_TYPE
// [EN] Set the screen with in pixels
// [ES] Indique el ancho de la pantalla en pixels

#define OLED_SCREEN_WIDTH 128

// [EN] Set the screen height in pixels
// [ES] Indique el alto de la pantalla en pixels

#define OLED_SCREEN_HEIGHT 64

// [EN] Set to true if the display is mounted upside down, false otherwise
// [ES] Indique true si la pantalla está montada del revés, false en otro caso

#define OLED_FLIP false

#endif // OLED_TYPE

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

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

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
    inputs::addRotaryEncoder(GPIO_NUM_36, GPIO_NUM_39); // fistButtonNumber=20
    inputs::addRotaryEncoder(GPIO_NUM_35, GPIO_NUM_32); // fistButtonNumber=22
    inputs::addRotaryEncoder(GPIO_NUM_25, GPIO_NUM_26); // fistButtonNumber=24
    inputs::addRotaryEncoder(GPIO_NUM_14, GPIO_NUM_18); // fistButtonNumber=26
    inputs::addDigital(GPIO_NUM_34, true, false);       // fistButtonNumber=27 (Rotary 1)
    inputs::addDigital(GPIO_NUM_33, true, false);       // fistButtonNumber=28 (Rotary 2)
    inputs::addDigital(GPIO_NUM_27, true, false);       // fistButtonNumber=29 (Rotary 3)

    inputHub::setDPADControls(19, 15, 11, 7);
    inputHub::setALTBitmap(BITMAP(0) | BITMAP(8));
    inputHub::setClutchPaddles(4, 12);
    inputHub::setClutchCalibrationButtons(21, 20); // Rotary 1
    inputHub::setMenuBitmap(BITMAP(27) | BITMAP(28));
    configMenu::setNavButtons(21, 20, 27, 28);
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

#ifdef OLED_TYPE
    language::begin();
    uiManager::begin();
    ui::begin(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, OLED_TYPE, OLED_FLIP);
#endif

    inputs::begin();
    inputHub::begin();
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