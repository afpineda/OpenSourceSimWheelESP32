/**
 * @file Setup7.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-02-11
 * @brief Sim wheel setup #7
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


// [EN] Set an output-capable GPIO number for the "wake up" pin.
//      Comment out if not required, or set an RTC-capable GPIO number for wake up.
// [ES] Indique el número de GPIO para la señal "despertar"
//      Comente la línea si no hay necesidad de entrar en sueño profundo, o bien,
//      indique un número de GPIO con capacidad RTC para despertar del sueño.

#define WAKE_UP_PIN GPIO_NUM_33

/* -----------------------------------------------------------------
 >>>> [EN] BATTERY MONITOR SUBSYSTEM
 >>>> [ES] SUBSISTEMA DE MONITORIZACIÓN DE BATERÍA
------------------------------------------------------------------ */

// [EN] Set an output-capable GPIO number for the "battEN" pin.
// [ES] Indique el número de GPIO para el pin "battEN".

// #define BATTERY_ENABLE_READ_GPIO GPIO_NUM_4

// [EN] Set an ADC-capable GPIO number for the "battREAD" pin.
// [ES] Indique el número de GPIO para el pin ADC de "battREAD".

// #define BATTERY_READ_GPIO -1

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------

void simWheelSetup()
{
    inputs::addShiftRegisters(GPIO_NUM_34, GPIO_NUM_48, GPIO_NUM_18, 25)
        //
        .inputNumber(0, sr8_pin_t::A, 9)
        .inputNumber(0, sr8_pin_t::B, 10)
        .inputNumber(0, sr8_pin_t::C, 15)
        .inputNumber(0, sr8_pin_t::D, 16)
        .inputNumber(0, sr8_pin_t::E, JOY_RB)
        .inputNumber(0, sr8_pin_t::F, 8)
        .inputNumber(0, sr8_pin_t::G, JOY_A)
        .inputNumber(0, sr8_pin_t::H, JOY_LB)
        //
        .inputNumber(1, sr8_pin_t::A, 11)
        .inputNumber(1, sr8_pin_t::B, 12)
        .inputNumber(1, sr8_pin_t::C, 13)
        .inputNumber(1, sr8_pin_t::D, 14)
        .inputNumber(1, sr8_pin_t::E, JOY_BACK)
        .inputNumber(1, sr8_pin_t::F, JOY_Y)
        .inputNumber(1, sr8_pin_t::G, JOY_X)
        .inputNumber(1, sr8_pin_t::H, JOY_B)
        //
        .inputNumber(2, sr8_pin_t::A, 17)
        .inputNumber(2, sr8_pin_t::B, 18)
        .inputNumber(2, sr8_pin_t::C, 24)
        .inputNumber(2, sr8_pin_t::D, JOY_START)
        .inputNumber(2, sr8_pin_t::E, 22)
        .inputNumber(2, sr8_pin_t::F, 21)
        .inputNumber(2, sr8_pin_t::G, 20)
        .inputNumber(2, sr8_pin_t::H, 19)
        .inputNumber(2, sr8_pin_t::SER, 23);

    inputs::addRotaryEncoder(GPIO_NUM_33, GPIO_NUM_39, ROT1_CW, ROT1_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_38, GPIO_NUM_37, ROT2_CW, ROT2_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_36, GPIO_NUM_35, ROT3_CW, ROT3_CCW);

    inputs::setAnalogClutchPaddles(GPIO_NUM_16, GPIO_NUM_17);
    inputHub::setClutchInputNumbers(LCLUTCH, RCLUTCH);
    inputHub::setClutchCalibrationInputNumbers(ROT1_CW, ROT1_CCW);
    inputHub::cycleCPWorkingMode_setInputNumbers({JOY_START, JOY_LB});
    inputHub::cmdRecalibrateAnalogAxis_setInputNumbers({(JOY_START), (JOY_LB), (JOY_RB)});
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