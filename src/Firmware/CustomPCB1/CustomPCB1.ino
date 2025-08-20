/**
 * @file CustomPCB1.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-08-19
 * @brief Firmware for custom ESP32-S3 DevKit board
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.hpp"
#include "SimWheelUI.hpp"

//------------------------------------------------------------------
// Hardware input and outputs (depends on board revision)
//------------------------------------------------------------------

// [EN] Assign your board revision number.
// [ES] Asigne el número de revisión de su placa.

#define BOARD_REVISION 2

//------------------------------------------------------------------

#if (BOARD_REVISION == 2)

#define ROT1A GPIO_NUM_4
#define ROT1B GPIO_NUM_5
#define ROT2A GPIO_NUM_7
#define ROT2B GPIO_NUM_6
#define ROT3A GPIO_NUM_11
#define ROT3B GPIO_NUM_10
#define ROT4A GPIO_NUM_13
#define ROT4B GPIO_NUM_12
#define ROT5A GPIO_NUM_17
#define ROT5B GPIO_NUM_18
#define ROT6A GPIO_NUM_16
#define ROT6B GPIO_NUM_21
#define ROT7A GPIO_NUM_15
#define ROT7B GPIO_NUM_33
#define ROT8A GPIO_NUM_14
#define ROT8B GPIO_NUM_34
#define DIN1 GPIO_NUM_40
#define DIN2 GPIO_NUM_38
#define LEFT_CLUTCH_GPIO GPIO_NUM_2
#define RIGHT_CLUTCH_GPIO GPIO_NUM_1
#define EXPANDER_Y_HW_ADDRESS 0
#define EXPANDER_X_HW_ADDRESS 7

#endif

//------------------------------------------------------------------
// Global customization
//------------------------------------------------------------------

/* -----------------------------------------------------------------
 >>>> [EN] DEVICE IDENTIFICATION
 >>>> [ES] IDENTIFICACIÓN DEL DISPOSITIVO
------------------------------------------------------------------ */

// [EN] Set a name for this device between double quotes
// [ES] Indique un nombre para este dispositivo entre comillas

std::string DEVICE_NAME = "Custom steering wheel";

// [EN] Set a manufacturer's name for this device between double quotes
// [ES] Indique un nombre para el fabricante de este dispositivo entre comillas

std::string DEVICE_MANUFACTURER = "Me";

/* -----------------------------------------------------------------
 >>>> [EN] DEEP SLEEP MODE
 >>>> [ES] MODO DE SUEÑO PROFUNDO
------------------------------------------------------------------ */

// [EN] Assign true if you have BLE and no
//      external power on/off switch
// [ES] Asigne true cuando use una implementación BLE
//      y no tenga un interruptor externo de apagado/encendido

// #define SHUTDOWN_ON_DISCONNECTION true
#define SHUTDOWN_ON_DISCONNECTION false

// [EN] Set a GPIO number for "wake up".
//      Comment out if not required, or set an RTC-capable GPIO number for wake up.
// [ES] Indique el número de GPIO para la señal "despertar"
//      Comente la línea si no hay necesidad de entrar en sueño profundo, o bien,
//      indique un número de GPIO con capacidad RTC para despertar del sueño.

#define WAKE_UP_PIN ROT1A

/* -----------------------------------------------------------------
 >>>> [EN] POWER LATCH SUBSYSTEM
 >>>> [ES] SUBSISTEMA DE CERROJO DE ENERGÍA
------------------------------------------------------------------ */

// [EN] Set an output-capable GPIO number for the "POWER_LATCH" pin.
//      Comment out if there is no external power latch circuit.
// [ES] Indique el número de GPIO para la señal "POWER_LATCH"
//      Comente la línea si no hay circuito externo de power latch.

// #define POWER_LATCH <here>

// [EN] Substitute <here> with a GPIO number or alias
// [ES] Sustituya <here> con un número de pin o su alias

// [EN] Set a latch mode
// [ES] Ajuste un mode de activación

#define LATCH_MODE PowerLatchMode::POWER_OPEN_DRAIN

// [EN] Set a delay (in milliseconds) to wait for the latch circuit
//      to do its magic (optional)
// [ES] Ajuste un retardo (en milisegundos) a esperar para
//      que el circuito haga su magia

#define LATCH_POWEROFF_DELAY 3000

/* -----------------------------------------------------------------
 >>>> [EN] DEVICE IDENTIFICATION
 >>>> [ES] IDENTIFICATION DEL DISPOSITIVO
------------------------------------------------------------------ */

// [EN] Uncomment the following lines to set a custom VID/PID
// [ES] Descomente la siguiente linea para ajustar el VID/PID a medida

// #define BLE_CUSTOM_VID <here>
// #define BLE_CUSTOM_PID <here>

// [EN] Substitute <here> with a non-zero 16-bits number as
//      a custom vendor/product ID
// [ES] Sustituya <here> con un número de 16 bits distinto de cero
//      como identificador de fabricante/producto a medida

/* -----------------------------------------------------------------
 >>>> [EN] LED STRIPS
 >>>> [ES] TIRAS LED
------------------------------------------------------------------ */

// [EN] Set the pixel count on each LED strip or set to zero
//      if there is no LED strip.
// [ES] Indique el número de pixels en cada tira LED o ponga cero
//      si no hay.

#define LED_STRIP1_COUNT 8
#define LED_STRIP2_COUNT 0

/* -----------------------------------------------------------------
 >>>> [EN] BATTERY SETTINGS
 >>>> [ES] AJUSTES DE BATERÍA
------------------------------------------------------------------ */

// [EN] Assign true if your device is powered by a battery
// [ES] Asigne true si su dispositivo está alimentado por una batería

#define ENABLE_BATTERY true
//#define ENABLE_BATTERY false

// [EN] Assign true if you want to shutdown before the battery depletes
// [ES] Asigne true si desea apagar el dispositivo antes de que
//      se agote la batería por completo

// #define SHUTDOWN_ON_LOW_BATTERY true
#define SHUTDOWN_ON_LOW_BATTERY false

//------------------------------------------------------------------
// Global input numbers (optional customization)
//------------------------------------------------------------------

#define GPIO_EXP_1ST 0
#define ROTARY_1ST 32
#define CLUTCH_1ST 48

//------------------------------------------------------------------
// Setup
//------------------------------------------------------------------

void simWheelSetup()
{
    // Configure LED strips

#if LED_STRIP1_COUNT > 0
    pixels::configure(
        PixelGroup::GRP_TELEMETRY,
        DIN1,
        LED_STRIP1_COUNT,
        true,
        PixelDriver::WS2812,
        PixelFormat::AUTO,
        127);
#endif

#if LED_STRIP2_COUNT > 0
    pixels::configure(
        PixelGroup::GRP_TELEMETRY,
        DIN2,
        LED_STRIP2_COUNT,
        true,
        PixelDriver::WS2812,
        PixelFormat::AUTO,
        127);
#endif

    // Configure rotary encoders

    inputs::addRotaryEncoder(ROT1A, ROT1B, ROTARY_1ST + 0, ROTARY_1ST + 1);
    inputs::addRotaryEncoder(ROT2A, ROT2B, ROTARY_1ST + 2, ROTARY_1ST + 3);
    inputs::addRotaryEncoder(ROT3A, ROT3B, ROTARY_1ST + 4, ROTARY_1ST + 5);
    inputs::addRotaryEncoder(ROT4A, ROT4B, ROTARY_1ST + 6, ROTARY_1ST + 7);
    inputs::addRotaryEncoder(ROT5A, ROT5B, ROTARY_1ST + 8, ROTARY_1ST + 9);
    inputs::addRotaryEncoder(ROT6A, ROT6B, ROTARY_1ST + 10, ROTARY_1ST + 11);
    inputs::addRotaryEncoder(ROT7A, ROT7B, ROTARY_1ST + 12, ROTARY_1ST + 13);
    inputs::addRotaryEncoder(ROT8A, ROT8B, ROTARY_1ST + 14, ROTARY_1ST + 15);

    // Configure GPIO expanders

    MCP23017Expander exp0, exp7;
    exp0[MCP23017Pin::GPA0] = GPIO_EXP_1ST;
    exp0[MCP23017Pin::GPA1] = GPIO_EXP_1ST + 1;
    exp0[MCP23017Pin::GPA2] = GPIO_EXP_1ST + 2;
    exp0[MCP23017Pin::GPA3] = GPIO_EXP_1ST + 3;
    exp0[MCP23017Pin::GPA4] = GPIO_EXP_1ST + 4;
    exp0[MCP23017Pin::GPA5] = GPIO_EXP_1ST + 5;
    exp0[MCP23017Pin::GPA6] = GPIO_EXP_1ST + 6;
    exp0[MCP23017Pin::GPA7] = GPIO_EXP_1ST + 7;
    exp0[MCP23017Pin::GPB0] = GPIO_EXP_1ST + 8;
    exp0[MCP23017Pin::GPB1] = GPIO_EXP_1ST + 9;
    exp0[MCP23017Pin::GPB2] = GPIO_EXP_1ST + 10;
    exp0[MCP23017Pin::GPB3] = GPIO_EXP_1ST + 11;
    exp0[MCP23017Pin::GPB4] = GPIO_EXP_1ST + 12;
    exp0[MCP23017Pin::GPB5] = GPIO_EXP_1ST + 13;
    exp0[MCP23017Pin::GPB6] = GPIO_EXP_1ST + 14;
    exp0[MCP23017Pin::GPB7] = GPIO_EXP_1ST + 15;

    exp7[MCP23017Pin::GPA0] = GPIO_EXP_1ST + 16;
    exp7[MCP23017Pin::GPA1] = GPIO_EXP_1ST + 16 + 1;
    exp7[MCP23017Pin::GPA2] = GPIO_EXP_1ST + 16 + 2;
    exp7[MCP23017Pin::GPA3] = GPIO_EXP_1ST + 16 + 3;
    exp7[MCP23017Pin::GPA4] = GPIO_EXP_1ST + 16 + 4;
    exp7[MCP23017Pin::GPA5] = GPIO_EXP_1ST + 16 + 5;
    exp7[MCP23017Pin::GPA6] = GPIO_EXP_1ST + 16 + 6;
    exp7[MCP23017Pin::GPA7] = GPIO_EXP_1ST + 16 + 7;
    exp7[MCP23017Pin::GPB0] = GPIO_EXP_1ST + 16 + 8;
    exp7[MCP23017Pin::GPB1] = GPIO_EXP_1ST + 16 + 9;
    exp7[MCP23017Pin::GPB2] = GPIO_EXP_1ST + 16 + 10;
    exp7[MCP23017Pin::GPB3] = GPIO_EXP_1ST + 16 + 11;
    exp7[MCP23017Pin::GPB4] = GPIO_EXP_1ST + 16 + 12;
    exp7[MCP23017Pin::GPB5] = GPIO_EXP_1ST + 16 + 13;
    exp7[MCP23017Pin::GPB6] = GPIO_EXP_1ST + 16 + 14;
    exp7[MCP23017Pin::GPB7] = GPIO_EXP_1ST + 16 + 15;

    inputs::addMCP23017Expander(exp0, EXPANDER_Y_HW_ADDRESS);
    inputs::addMCP23017Expander(exp7, EXPANDER_X_HW_ADDRESS);

    // Configure clutch paddles

    inputs::setAnalogClutchPaddles(LEFT_CLUTCH_GPIO, RIGHT_CLUTCH_GPIO);
    inputHub::clutch::inputs(CLUTCH_1ST, CLUTCH_1ST + 1);
    inputHub::clutch::bitePointInputs(ROTARY_1ST, ROTARY_1ST + 1);

    // Default input map

    inputMap::setOptimal();
}

//------------------------------------------------------------------

void customFirmware()
{

#ifdef WAKE_UP_PIN
    power::configureWakeUp(ROT1A);
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
        DEVICE_MANUFACTURER,
        SHUTDOWN_ON_DISCONNECTION
#if defined(BLE_CUSTOM_VID)
        ,
        BLE_CUSTOM_VID
#if defined(BLE_CUSTOM_PID)
        ,
        BLE_CUSTOM_PID
#endif
#endif
    );

#if ENABLE_BATTERY
    batteryMonitor::configure();
#if !SHUTDOWN_ON_LOW_BATTERY
    batteryMonitor::setPowerOffSoC(0);
#endif
    batteryMonitor::setWarningSoC(15);
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