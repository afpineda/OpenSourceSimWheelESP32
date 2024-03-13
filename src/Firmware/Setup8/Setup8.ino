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

//#include <Arduino.h>
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
    inputs::addShiftRegisters(GPIO_NUM_39, GPIO_NUM_33, GPIO_NUM_34, 25)
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

    inputs::addRotaryEncoder(GPIO_NUM_38, GPIO_NUM_37, ROT1_CW, ROT1_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_36, GPIO_NUM_35, ROT2_CW, ROT2_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_48, GPIO_NUM_18, ROT3_CW, ROT3_CCW);
    inputs::addRotaryEncoder(GPIO_NUM_17, GPIO_NUM_16, ROT4_CW, ROT4_CCW);

    // LilyGo T-QT built-in push buttons
    inputs::addDigital(GPIO_NUM_0,60);
    inputs::addDigital(GPIO_NUM_47,61);
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    simWheelSetup();
    hidImplementation::begin(
        DEVICE_NAME,
        DEVICE_MANUFACTURER,
        false);
    inputs::start();
}

void loop()
{
    vTaskDelay(portMAX_DELAY);
}