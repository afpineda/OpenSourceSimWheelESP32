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
    ShiftRegisterChip chip1, chip2, chip3;
    //
    chip1[SR8Pin::A] = 9;
    chip1[SR8Pin::B] = 10;
    chip1[SR8Pin::C] = 15;
    chip1[SR8Pin::D] = 16;
    chip1[SR8Pin::E] = JOY_RB;
    chip1[SR8Pin::F] = 8;
    chip1[SR8Pin::G] = JOY_A;
    chip1[SR8Pin::H] = JOY_LB;
    //
    chip2[SR8Pin::A] = 11;
    chip2[SR8Pin::B] = 12;
    chip2[SR8Pin::C] = 13;
    chip2[SR8Pin::D] = 14;
    chip2[SR8Pin::E] = JOY_BACK;
    chip2[SR8Pin::F] = JOY_Y;
    chip2[SR8Pin::G] = JOY_X;
    chip2[SR8Pin::H] = JOY_B;
    //
    chip3[SR8Pin::A] = 17;
    chip3[SR8Pin::B] = 18;
    chip3[SR8Pin::C] = 24;
    chip3[SR8Pin::D] = JOY_START;
    chip3[SR8Pin::E] = 22;
    chip3[SR8Pin::F] = 21;
    chip3[SR8Pin::G] = 20;
    chip3[SR8Pin::H] = 19;

    inputs::add74HC165NChain(
        GPIO_NUM_48,
        GPIO_NUM_18,
        GPIO_NUM_34,
        {chip1, chip2, chip3},
        23);

    inputs::addRotaryEncoder(GPIO_NUM_33, GPIO_NUM_39, ROT1_CW, ROT1_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_38, GPIO_NUM_37, ROT2_CW, ROT2_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_36, GPIO_NUM_35, ROT3_CW, ROT3_CCW);

    inputs::setAnalogClutchPaddles(GPIO_NUM_16, GPIO_NUM_17);
    inputHub::clutch::inputs(LCLUTCH, RCLUTCH);
    inputHub::clutch::bitePointInputs(ROT1_CW, ROT1_CCW);
    inputHub::clutch::cycleWorkingModeInputs({JOY_START, JOY_LB});
    inputHub::clutch::cmdRecalibrateAxisInputs({(JOY_START), (JOY_LB), (JOY_RB)});
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