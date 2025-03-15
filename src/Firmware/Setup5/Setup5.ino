/**
 * @file Setup5.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-02-11
 * @brief Sim wheel setup #5
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
#define LCLUTCH 30
#define RCLUTCH 31

/* -----------------------------------------------------------------
 >>>> [EN] DEVICE IDENTIFICATION
 >>>> [ES] IDENTIFICACIÓN DEL DISPOSITIVO
------------------------------------------------------------------ */

// [EN] Set a name for this device between double quotes
// [ES] Indique un nombre para este dispositivo entre comillas

std::string DEVICE_NAME = "Open steering wheel-5";

// [EN] Set a manufacturer's name for this device between double quotes
// [ES] Indique un nombre para el fabricante de este dispositivo entre comillas

std::string DEVICE_MANUFACTURER = "Mamandurrio";

/* -----------------------------------------------------------------
 >>>> [EN] DEEP SLEEP MODE
 >>>> [ES] MODO DE SUEÑO PROFUNDO
------------------------------------------------------------------ */

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
    AnalogMultiplexerChip8 chip1(GPIO_NUM_16);
    chip1[Mux8Pin::A0] = JOY_BACK;
    chip1[Mux8Pin::A1] = JOY_LB;
    chip1[Mux8Pin::A2] = LCLUTCH;
    chip1[Mux8Pin::A3] = JOY_A;
    chip1[Mux8Pin::A4] = RCLUTCH;
    chip1[Mux8Pin::A5] = JOY_B;
    chip1[Mux8Pin::A6] = JOY_RB;
    chip1[Mux8Pin::A7] = JOY_START;

    AnalogMultiplexerChip8 chip2(GPIO_NUM_34);
    chip2[Mux8Pin::A0] = 10;
    chip2[Mux8Pin::A1] = 8;
    chip2[Mux8Pin::A2] = JOY_X;
    chip2[Mux8Pin::A3] = 12;
    chip2[Mux8Pin::A4] = JOY_Y;
    chip2[Mux8Pin::A5] = 13;
    chip2[Mux8Pin::A6] = 9;
    chip2[Mux8Pin::A7] = 11;
    inputs::addAnalogMultiplexerGroup(
        GPIO_NUM_48,
        GPIO_NUM_18,
        GPIO_NUM_17,
        {chip1, chip2});

    inputs::addRotaryEncoder(GPIO_NUM_33, GPIO_NUM_39, ROT1_CW, ROT1_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_38, GPIO_NUM_37, ROT2_CW, ROT2_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_36, GPIO_NUM_35, ROT3_CW, ROT3_CCW);

    inputHub::clutch::inputs(LCLUTCH, RCLUTCH);
    inputHub::clutch::bitePointInputs(ROT1_CW, ROT1_CCW);
    inputHub::clutch::cycleWorkingModeInputs({JOY_START, JOY_LB});
}

//------------------------------------------------------------------

void customFirmware()
{
#ifdef WAKE_UP_PIN
    power::configureWakeUp(WAKE_UP_PIN);
#endif

#ifdef POWER_LATCH
    power::configureWakeUp(
        POWER_LATCH,
        LATCH_MODE,
        LATCH_POWEROFF_DELAY);
#endif

    simWheelSetup();
    hid::configure(
        DEVICE_NAME,
        DEVICE_MANUFACTURER);

#ifdef ENABLE_BATTERY_MONITOR
    batteryMonitor::begin(
        BATTERY_READ_GPIO,
        BATTERY_ENABLE_READ_GPIO, );
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