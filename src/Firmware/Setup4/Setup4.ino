/**
 * @file Setup4.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-01-20
 * @brief Sim wheel setup #4
 *
 * @copyright Licensed under the EUPL
 *
 */

// #include <Arduino.h>
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
#define ALT1 20
#define ALT2 21
#define LCLUTCH 30
#define RCLUTCH 31

/* -----------------------------------------------------------------
 >>>> [EN] DEVICE IDENTIFICATION
 >>>> [ES] IDENTIFICACIÓN DEL DISPOSITIVO
------------------------------------------------------------------ */

// [EN] Set a name for this device between double quotes
// [ES] Indique un nombre para este dispositivo entre comillas

std::string DEVICE_NAME = "Open steering wheel-4";

// [EN] Set a manufacturer's name for this device between double quotes
// [ES] Indique un nombre para el fabricante de este dispositivo entre comillas

std::string DEVICE_MANUFACTURER = "Mamandurrio";

/* -----------------------------------------------------------------
 >>>> [EN] DEEP SLEEP MODE
 >>>> [ES] MODO DE SUEÑO PROFUNDO
------------------------------------------------------------------ */

// [EN] Set an output-capable GPIO number for the "wake up" pin.
//      Comment out if not required, or set an RTC-capable GPIO number for wake up.
// [ES] Indique el número de GPIO para la señal "despertar"
//      Comente la línea si no hay necesidad de entrar en sueño profundo, o bien,
//      indique un número de GPIO con capacidad RTC para despertar del sueño.

#define WAKE_UP_PIN GPIO_NUM_27

/* -----------------------------------------------------------------
 >>>> [EN] BATTERY MONITOR SUBSYSTEM
 >>>> [ES] SUBSISTEMA DE MONITORIZACIÓN DE BATERÍA
------------------------------------------------------------------ */

// [EN] Set an output-capable GPIO number for the "battEN" pin.
// [ES] Indique el número de GPIO para el pin "battEN".

#define BATTERY_ENABLE_READ_GPIO GPIO_NUM_35

// [EN] Set an ADC-capable GPIO number for the "battREAD" pin.
// [ES] Indique el número de GPIO para el pin ADC de "battREAD".

#define BATTERY_READ_GPIO -1

/* -----------------------------------------------------------------
 >>>> [EN] MULTIPLEXED BUTTONS
 >>>> [ES] BOTONES MULTIPLEXADOS
------------------------------------------------------------------ */

// [EN] Set all GPIO numbers for selector pins between curly brackets
// [ES] Indique los números de GPIO de todos los pines selectores entre las llaves
static const gpio_num_array_t mpxSelectors = {GPIO_NUM_27, GPIO_NUM_15, GPIO_NUM_14};

// [EN] Set all GPIO numbers for input pins between curly brackets
// [ES] Indique los números de GPIO de todos los pines de entrada entre las llaves
static const gpio_num_array_t mpxInputs = {GPIO_NUM_32, GPIO_NUM_33};

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------

void simWheelSetup()
{
    inputs::addAnalogMultiplexer(mpxSelectors, mpxInputs)
        //
        .inputNumber(mpxInputs[0], mux8_pin_t::A0, 9)
        .inputNumber(mpxInputs[0], mux8_pin_t::A1, 8)
        .inputNumber(mpxInputs[0], mux8_pin_t::A2, JOY_BACK)
        .inputNumber(mpxInputs[0], mux8_pin_t::A3, 10)
        .inputNumber(mpxInputs[0], mux8_pin_t::A4, JOY_A)
        .inputNumber(mpxInputs[0], mux8_pin_t::A5, JOY_Y)
        .inputNumber(mpxInputs[0], mux8_pin_t::A6, JOY_B)
        .inputNumber(mpxInputs[0], mux8_pin_t::A7, JOY_X)
        //
        .inputNumber(mpxInputs[1], mux8_pin_t::A0, 12)
        .inputNumber(mpxInputs[1], mux8_pin_t::A1, 13)
        .inputNumber(mpxInputs[1], mux8_pin_t::A2, 14)
        .inputNumber(mpxInputs[1], mux8_pin_t::A3, 11)
        .inputNumber(mpxInputs[1], mux8_pin_t::A4, JOY_RB)
        .inputNumber(mpxInputs[1], mux8_pin_t::A5, JOY_LB)
        .inputNumber(mpxInputs[1], mux8_pin_t::A6, ALT2)
        .inputNumber(mpxInputs[1], mux8_pin_t::A7, ALT1);

    inputs::addDigital(GPIO_NUM_4, JOY_START);

    inputs::addRotaryEncoder(GPIO_NUM_22, GPIO_NUM_21, ROT1_CW, ROT1_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_5, GPIO_NUM_18, ROT2_CW, ROT2_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_23, GPIO_NUM_19, ROT3_CW, ROT3_CCW);

    inputs::setAnalogClutchPaddles(GPIO_NUM_25, GPIO_NUM_26);
    inputHub::setClutchInputNumbers(LCLUTCH, RCLUTCH);
    inputHub::setClutchCalibrationInputNumbers(ROT1_CW, ROT1_CCW);
    inputHub::cycleCPWorkingMode_setInputNumbers({JOY_START, JOY_LB});
    inputHub::cmdRecalibrateAnalogAxis_setInputNumbers({JOY_RB, JOY_LB, JOY_START});

    inputHub::setALTInputNumbers({ALT1, ALT2});
    inputHub::cycleALTButtonsWorkingMode_setInputNumbers({JOY_START, JOY_RB});
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);

    userSettings::begin();
    power::begin((gpio_num_t)WAKE_UP_PIN);
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