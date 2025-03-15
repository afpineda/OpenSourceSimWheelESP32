/**
 * @file Setup8.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-10-25
 * @brief Sim wheel setup #8
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
#define ROT4_CW 46
#define ROT4_CCW 47

/* -----------------------------------------------------------------
 >>>> [EN] DEVICE IDENTIFICATION
 >>>> [ES] IDENTIFICACIÓN DEL DISPOSITIVO
------------------------------------------------------------------ */

// [EN] Set a name for this device between double quotes
// [ES] Indique un nombre para este dispositivo entre comillas

std::string DEVICE_NAME = "ButtonBox-8";

// [EN] Set a manufacturer's name for this device between double quotes
// [ES] Indique un nombre para el fabricante de este dispositivo entre comillas

std::string DEVICE_MANUFACTURER = "Mamandurrio";

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
        GPIO_NUM_34,
        GPIO_NUM_48,
        GPIO_NUM_18,
        {chip1, chip2, chip3},
        23);

    inputs::addRotaryEncoder(GPIO_NUM_38, GPIO_NUM_37, ROT1_CW, ROT1_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_36, GPIO_NUM_35, ROT2_CW, ROT2_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_48, GPIO_NUM_18, ROT3_CW, ROT3_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_17, GPIO_NUM_16, ROT4_CW, ROT4_CCW);

    // LilyGo T-QT built-in push buttons
    inputs::addButton(GPIO_NUM_0, 60);
    inputs::addButton(GPIO_NUM_47, 61);
}

//------------------------------------------------------------------

void customFirmware()
{
    simWheelSetup();
    hid::configure(
        DEVICE_NAME,
        DEVICE_MANUFACTURER,
        false);
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