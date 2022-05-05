/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-01
 * @brief Sim wheel setup #1
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "SimWheel.h"

//------------------------------------------------------------------
// Global customization
//------------------------------------------------------------------

/* -----------------------------------------------------------------
 >>>> [EN] DEVICE IDENTIFICATION
 >>>> [ES] IDENTIFICACIÓN DEL DISPOSITIVO
------------------------------------------------------------------ */

std::string DEVICE_NAME = "Open steering wheel-1";
std::string DEVICE_MANUFACTURER = "Mamandurrio";

/* -----------------------------------------------------------------
 >>>> [EN] DEEP SLEEP MODE
 >>>> [ES] MODO DE SUEÑO PROFUNDO
------------------------------------------------------------------ */

const gpio_num_t WAKEUP_PINS[] = {GPIO_NUM_39, GPIO_NUM_32};
#define WAKEUP_ANYorALL false

/* -----------------------------------------------------------------
 >>>> [EN] POWER LATCH SUBSYSTEM
 >>>> [ES] SUBSISTEMA DE CERROJO DE ENERGÍA
------------------------------------------------------------------ */

#define POWER_LATCH GPIO_NUM_1
const powerLatchMode_t LATCH_MODE = POWER_OPEN_DRAIN;
#define LATCH_POWEROFF_DELAY 3000

/* -----------------------------------------------------------------
 >>>> OLED
------------------------------------------------------------------ */

#define OLED_TYPE SSOLED_132x64
#define OLED_SCREEN_WIDTH 128
#define OLED_SCREEN_HEIGHT 64
#define OLED_FLIP false

/* -----------------------------------------------------------------
 >>>> [EN] BATTERY MONITOR SUBSYSTEM
 >>>> [ES] SUBSISTEMA DE MONITORIZACIÓN DE BATERÍA
------------------------------------------------------------------ */

#define ENABLE_BATTERY_MONITOR
#define BATTERY_ENABLE_READ_GPIO GPIO_NUM_12
#define BATTERY_READ_GPIO GPIO_NUM_2

/* -----------------------------------------------------------------
 >>>> [EN] BUTTON MATRIX
 >>>> [ES] MATRIZ DE BOTONES
------------------------------------------------------------------ */

static const gpio_num_t mtxSelectors[] = {
    GPIO_NUM_0,
    GPIO_NUM_16,
    GPIO_NUM_17,
    GPIO_NUM_5,
    GPIO_NUM_18};
static const gpio_num_t mtxInputs[] = {
    GPIO_NUM_15,
    GPIO_NUM_4,
    GPIO_NUM_19, 
    GPIO_NUM_23, 
    GPIO_NUM_26};

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------

void simWheelSetup()
{
    inputs::setButtonMatrix(
        mtxSelectors,
        sizeof(mtxSelectors) / sizeof(mtxSelectors[0]),
        mtxInputs,
        sizeof(mtxInputs) / sizeof(mtxInputs[0]));      // fistButtonNumber=0
    inputs::addRotaryEncoder(GPIO_NUM_36, GPIO_NUM_34); // fistButtonNumber=25
    inputs::addRotaryEncoder(GPIO_NUM_35, GPIO_NUM_33); // fistButtonNumber=27
    inputs::addRotaryEncoder(GPIO_NUM_25, GPIO_NUM_27); // fistButtonNumber=29
    inputs::addRotaryEncoder(GPIO_NUM_14, GPIO_NUM_13); // fistButtonNumber=31
    inputs::addDigital(GPIO_NUM_39, true, false);       // fistButtonNumber=33 (Rotary 1)
    inputs::addDigital(GPIO_NUM_32, true, false);       // fistButtonNumber=34 (Rotary 2)
    inputs::addDigital(GPIO_NUM_3, true, false);        // fistButtonNumber=35 (Rotary 3)

    inputHub::setDPADControls(16, 17, 18, 19);
    inputHub::setALTBitmap(BITMAP(20) | BITMAP(21));
    inputHub::setClutchPaddles(23, 24);
    inputHub::setClutchCalibrationButtons(25, 26); // Rotary 1
    inputHub::setMenuBitmap(BITMAP(33) | BITMAP(34));
    configMenu::setNavButtons(26, 25, 33, 34);
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    power::begin(
        WAKEUP_PINS,
        sizeof(WAKEUP_PINS) / sizeof(gpio_num_t),
        WAKEUP_ANYorALL);

    power::setPowerLatch(
        (gpio_num_t)POWER_LATCH,
        LATCH_MODE,
        LATCH_POWEROFF_DELAY);

    language::begin();
    uiManager::begin();
    ui::begin(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, OLED_TYPE, OLED_FLIP);

    inputs::begin();
    inputHub::begin();
    simWheelSetup();
    hidImplementation::begin(
        DEVICE_NAME,
        DEVICE_MANUFACTURER);

    batteryCalibration::begin();
    power::startBatteryMonitor(
        (gpio_num_t)BATTERY_ENABLE_READ_GPIO,
        (gpio_num_t)BATTERY_READ_GPIO);
    inputs::start();
}

void loop()
{
    vTaskDelay(portMAX_DELAY);
}