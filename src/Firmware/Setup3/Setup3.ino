/**
 * @file Setup3.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-01-13
 * @brief Sim wheel setup #3
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.hpp"
#include "SimWheelUI.hpp"

//------------------------------------------------------------------
// Global customization
//------------------------------------------------------------------

#define ROT1_CW 40
#define ROT1_CCW 41
#define ROT2_CW 42
#define ROT2_CCW 43
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

std::string DEVICE_NAME = "Open steering wheel-3";

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

#define BATTERY_ENABLE_READ_GPIO -1

// [EN] Set an ADC-capable GPIO number for the "battREAD" pin.
// [ES] Indique el número de GPIO para el pin ADC de "battREAD".

#define BATTERY_READ_GPIO GPIO_NUM_35

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------

void simWheelSetup()
{
    ButtonMatrix mtx;
    // GPIO_NUM_23
    mtx[GPIO_NUM_22][GPIO_NUM_23] = JOY_RB;
    mtx[GPIO_NUM_21][GPIO_NUM_23] = ALT2;
    mtx[GPIO_NUM_32][GPIO_NUM_23] = JOY_B;
    mtx[GPIO_NUM_33][GPIO_NUM_23] = JOY_Y;
    // GPIO_NUM_19
    mtx[GPIO_NUM_22][GPIO_NUM_19] = JOY_LB;
    mtx[GPIO_NUM_21][GPIO_NUM_19] = ALT1;
    mtx[GPIO_NUM_32][GPIO_NUM_19] = JOY_A;
    mtx[GPIO_NUM_33][GPIO_NUM_19] = JOY_X;
    // GPIO_NUM_18
    mtx[GPIO_NUM_22][GPIO_NUM_18] = 13;
    mtx[GPIO_NUM_21][GPIO_NUM_18] = 11;
    mtx[GPIO_NUM_32][GPIO_NUM_18] = 9;
    mtx[GPIO_NUM_33][GPIO_NUM_18] = JOY_START;
    // GPIO_NUM_5
    mtx[GPIO_NUM_22][GPIO_NUM_5] = 12;
    mtx[GPIO_NUM_21][GPIO_NUM_5] = 10;
    mtx[GPIO_NUM_32][GPIO_NUM_5] = 8;
    mtx[GPIO_NUM_33][GPIO_NUM_5] = JOY_BACK;
    inputs::addButtonMatrix(mtx);

    inputs::addRotaryEncoder(GPIO_NUM_26, GPIO_NUM_27, ROT1_CW, ROT1_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_14, GPIO_NUM_4, ROT2_CW, ROT2_CCW);
    inputs::setAnalogClutchPaddles(GPIO_NUM_25, GPIO_NUM_26);
    inputHub::clutch::inputs(LCLUTCH, RCLUTCH);
    inputHub::altButtons::inputs({ALT1, ALT2});
    inputHub::clutch::bitePointInputs(ROT1_CW, ROT1_CCW);
    inputHub::clutch::cycleWorkingModeInputs({JOY_START, JOY_LB});
    inputHub::altButtons::cycleWorkingModeInputs({JOY_START, JOY_RB});
    inputHub::clutch::cmdRecalibrateAxisInputs({JOY_RB, JOY_LB, JOY_START});
}

//------------------------------------------------------------------

void customFirmware()
{
    power::configureWakeUp(WAKE_UP_PIN);
    simWheelSetup();
    hid::configure(
        DEVICE_NAME,
        DEVICE_MANUFACTURER);

    batteryMonitor::configure(
        (gpio_num_t)BATTERY_ENABLE_READ_GPIO,
        (gpio_num_t)BATTERY_READ_GPIO);
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