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

// #include <Arduino.h>
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

// [EN] Set an output-capable GPIO number for the "wake up" pin.
//      Comment out if not required, or set an RTC-capable GPIO number for wake up.
// [ES] Indique el número de GPIO para la señal "despertar"
//      Comente la línea si no hay necesidad de entrar en sueño profundo, o bien,
//      indique un número de GPIO con capacidad RTC para despertar del sueño.

#define WAKE_UP_PIN GPIO_NUM_32

/* -----------------------------------------------------------------
 >>>> [EN] MULTIPLEXED BUTTONS
 >>>> [ES] BOTONES MULTIPLEXADOS
------------------------------------------------------------------ */

// [EN] Set all GPIO numbers for selector pins between curly brackets
// [ES] Indique los números de GPIO de todos los pines selectores entre las llaves
static const gpio_num_array_t mpxSelectors = {GPIO_NUM_16, GPIO_NUM_17, GPIO_NUM_5};

// [EN] Set all GPIO numbers for input pins between curly brackets
// [ES] Indique los números de GPIO de todos los pines de entrada entre las llaves
static const gpio_num_array_t mpxInputs = {GPIO_NUM_14, GPIO_NUM_22};

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------

void simWheelSetup()
{
    inputs::addAnalogMultiplexer(mpxSelectors, mpxInputs)
        //
        .inputNumber(mpxInputs[0], mux8_pin_t::A0, 10)
        .inputNumber(mpxInputs[0], mux8_pin_t::A1, JOY_BACK)
        .inputNumber(mpxInputs[0], mux8_pin_t::A2, JOY_A)
        .inputNumber(mpxInputs[0], mux8_pin_t::A3, 14)
        .inputNumber(mpxInputs[0], mux8_pin_t::A4, ALT2)
        .inputNumber(mpxInputs[0], mux8_pin_t::A5, 11)
        .inputNumber(mpxInputs[0], mux8_pin_t::A6, 15)
        .inputNumber(mpxInputs[0], mux8_pin_t::A7, 13)
        //
        .inputNumber(mpxInputs[1], mux8_pin_t::A0, JOY_Y)
        .inputNumber(mpxInputs[1], mux8_pin_t::A1, JOY_START)
        .inputNumber(mpxInputs[1], mux8_pin_t::A2, 9)
        .inputNumber(mpxInputs[1], mux8_pin_t::A3, JOY_B)
        .inputNumber(mpxInputs[1], mux8_pin_t::A4, ALT1)
        .inputNumber(mpxInputs[1], mux8_pin_t::A5, JOY_X)
        .inputNumber(mpxInputs[1], mux8_pin_t::A6, 12)
        .inputNumber(mpxInputs[1], mux8_pin_t::A7, 8);

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
    inputHub::setALTInputNumbers({ALT1, ALT2});
    inputHub::setClutchCalibrationInputNumbers(ROT1_CW, ROT1_CCW);
    inputHub::cycleCPWorkingMode_setInputNumbers({ROT1_SW, JOY_LB});
    inputHub::cycleALTButtonsWorkingMode_setInputNumbers({ROT1_SW, JOY_RB});
    inputHub::cmdRecalibrateAnalogAxis_setInputNumbers({ROT1_SW, JOY_RB, JOY_LB});
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