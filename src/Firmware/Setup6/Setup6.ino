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

#include "SimWheel.hpp"
#include "SimWheelUI.hpp"

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

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------

void simWheelSetup()
{
    AnalogMultiplexerChip8 chip1(GPIO_NUM_14);
    chip1[Mux8Pin::A0] = 10;
    chip1[Mux8Pin::A1] = JOY_BACK;
    chip1[Mux8Pin::A2] = JOY_A;
    chip1[Mux8Pin::A3] = 14;
    chip1[Mux8Pin::A4] = ALT2;
    chip1[Mux8Pin::A5] = 11;
    chip1[Mux8Pin::A6] = 15;
    chip1[Mux8Pin::A7] = 13;

    AnalogMultiplexerChip8 chip2(GPIO_NUM_22);
    chip2[Mux8Pin::A0] = JOY_Y;
    chip2[Mux8Pin::A1] = JOY_START;
    chip2[Mux8Pin::A2] = 9;
    chip2[Mux8Pin::A3] = JOY_B;
    chip2[Mux8Pin::A4] = ALT1;
    chip2[Mux8Pin::A5] = JOY_X;
    chip2[Mux8Pin::A6] = 12;
    chip2[Mux8Pin::A7] = 8;
    inputs::addAnalogMultiplexerGroup(
        GPIO_NUM_16,
        GPIO_NUM_17,
        GPIO_NUM_5,
        {chip1, chip2});

    inputs::addRotaryEncoder(GPIO_NUM_32, GPIO_NUM_33, ROT1_CW, ROT1_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_25, GPIO_NUM_26, ROT2_CW, ROT2_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_27, GPIO_NUM_12, ROT3_CW, ROT3_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_13, GPIO_NUM_15, ROT4_CW, ROT4_CCW);

    inputs::addButton(GPIO_NUM_2, ROT1_SW);
    inputs::addButton(GPIO_NUM_0, ROT2_SW);
    inputs::addButton(GPIO_NUM_4, ROT3_SW);
    inputs::addButton(GPIO_NUM_18, ROT4_SW);
    inputs::addButton(GPIO_NUM_23, JOY_LSHIFT_PADDLE);
    inputs::addButton(GPIO_NUM_19, JOY_RSHIFT_PADDLE);

    inputs::setAnalogClutchPaddles(GPIO_NUM_36, GPIO_NUM_39);
    inputHub::clutch::inputs(LCLUTCH, RCLUTCH);
    inputHub::altButtons::inputs({ALT1, ALT2});
    inputHub::clutch::bitePointInputs(ROT1_CW, ROT1_CCW);
    inputHub::clutch::cycleWorkingModeInputs({ROT1_SW, JOY_LB});
    inputHub::altButtons::cycleWorkingModeInputs({ROT1_SW, JOY_RB});
    inputHub::clutch::cmdRecalibrateAxisInputs({ROT1_SW, JOY_RB, JOY_LB});
}

//------------------------------------------------------------------

void customFirmware()
{
    power::configureWakeUp(WAKE_UP_PIN);
    simWheelSetup();
    hid::configure(
        DEVICE_NAME,
        DEVICE_MANUFACTURER);
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