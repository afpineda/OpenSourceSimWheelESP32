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

#include "SimWheel.hpp"
#include "SimWheelUI.hpp"

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

// [EN] Comment out the following line if the battery monitor subsystem is NOT in place
// [ES] Comentar la siguiente linea si NO hay subsistema de monitorización de batería
#define ENABLE_BATTERY_MONITOR

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
    AnalogMultiplexerChip8 chip1(GPIO_NUM_32);
    chip1[Mux8Pin::A0] = 9;
    chip1[Mux8Pin::A1] = 8;
    chip1[Mux8Pin::A2] = JOY_BACK;
    chip1[Mux8Pin::A3] = 10;
    chip1[Mux8Pin::A4] = JOY_A;
    chip1[Mux8Pin::A5] = JOY_Y;
    chip1[Mux8Pin::A6] = JOY_B;
    chip1[Mux8Pin::A7] = JOY_X;

    AnalogMultiplexerChip8 chip2(GPIO_NUM_33);
    chip2[Mux8Pin::A0] = 12;
    chip2[Mux8Pin::A1] = 13;
    chip2[Mux8Pin::A2] = 14;
    chip2[Mux8Pin::A3] = 11;
    chip2[Mux8Pin::A4] = JOY_RB;
    chip2[Mux8Pin::A5] = JOY_LB;
    chip2[Mux8Pin::A6] = ALT2;
    chip2[Mux8Pin::A7] = ALT1;

    inputs::addAnalogMultiplexerGroup(
        GPIO_NUM_27,
        GPIO_NUM_15,
        GPIO_NUM_14,
        {chip1, chip2});

    inputs::addButton(GPIO_NUM_4, JOY_START);

    inputs::addRotaryEncoder(GPIO_NUM_22, GPIO_NUM_21, ROT1_CW, ROT1_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_5, GPIO_NUM_18, ROT2_CW, ROT2_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_23, GPIO_NUM_19, ROT3_CW, ROT3_CCW);

    inputs::setAnalogClutchPaddles(GPIO_NUM_25, GPIO_NUM_26);

    inputHub::clutch::inputs(LCLUTCH, RCLUTCH);
    inputHub::clutch::bitePointInputs(ROT1_CW, ROT1_CCW);
    inputHub::clutch::cycleWorkingModeInputs({JOY_START, JOY_LB});
    inputHub::clutch::cmdRecalibrateAxisInputs({JOY_RB, JOY_LB, JOY_START});

    inputHub::altButtons::inputs({ALT1, ALT2});
    inputHub::altButtons::cycleWorkingModeInputs({JOY_START, JOY_RB});
}

//------------------------------------------------------------------

void customFirmware()
{
    power::configureWakeUp(WAKE_UP_PIN);
    simWheelSetup();
    hid::configure(
        DEVICE_NAME,
        DEVICE_MANUFACTURER);

#ifdef ENABLE_BATTERY_MONITOR
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