/**
 * @file Teabox.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-02-15
 * @brief Sim wheel setup "teabox"
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.hpp"

//------------------------------------------------------------------
// Global customization
//------------------------------------------------------------------

#define ROT1_SW JOY_A
#define BTN1 JOY_B
#define BTN2 JOY_X
#define BTN3 JOY_Y
#define ROT1_CW JOY_LTHUMBSTICK_CLICK
#define ROT1_CCW JOY_RTHUMBSTICK_CLICK
#define BTN_UP JOY_LB
#define BTN_DOWN JOY_RB

/* -----------------------------------------------------------------
 >>>> [EN] DEVICE IDENTIFICATION
 >>>> [ES] IDENTIFICACIÓN DEL DISPOSITIVO
------------------------------------------------------------------ */

// [EN] Set a name for this device between double quotes
// [ES] Indique un nombre para este dispositivo entre comillas

std::string DEVICE_NAME = "Teabox";

// [EN] Set a manufacturer's name for this device between double quotes
// [ES] Indique un nombre para el fabricante de este dispositivo entre comillas

std::string DEVICE_MANUFACTURER = "Mamandurrio";

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------

void simWheelSetup()
{
    inputs::addRotaryEncoder(GPIO_NUM_12, GPIO_NUM_14, ROT1_CW, ROT1_CCW);
    inputs::addButton(GPIO_NUM_27, ROT1_SW);
    inputs::addButton(GPIO_NUM_26, BTN_UP);
    inputs::addButton(GPIO_NUM_25, BTN_DOWN);
    inputs::addButton(GPIO_NUM_13, BTN1);
    inputs::addButton(GPIO_NUM_15, BTN2);
    inputs::addButton(GPIO_NUM_2, BTN3);

    inputHub::dpad::inputs(BTN_UP, BTN_DOWN, ROT1_CCW, ROT1_CW);

    inputHub::securityLock::cycleWorkingModeInputs({BTN1, BTN2, BTN3});
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