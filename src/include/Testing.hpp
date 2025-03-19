/**
 * @file Testing.hpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-11
 * @brief Global configuration for testing in Arduino IDE
 *
 * @copyright Licensed under the EUPL
 *
 */

#pragma once

// ------------------------------------------------------------
// Imports
// ------------------------------------------------------------

#include "InputSpecification.hpp"

/// @cond

// ------------------------------------------------------------
// ------------------------------------------------------------
// Pin-out plan
// ------------------------------------------------------------
// ------------------------------------------------------------

// -------------------------------------------------------------
#if defined(CONFIG_IDF_TARGET_ESP32)
// ESP32 -------------------------------------------------------

// Button matrix pins for testing
#define TEST_BTNMTX_ROW1 GPIO_NUM_12
#define TEST_BTNMTX_ROW2 GPIO_NUM_13
#define TEST_BTNMTX_COL1 GPIO_NUM_26
#define TEST_BTNMTX_COL2 GPIO_NUM_27
#define TEST_BTNMTX_COL3 GPIO_NUM_14

// Relative rotary encoder pins for testing
#define TEST_ROTARY_SW GPIO_NUM_34
#define TEST_ROTARY_CLK GPIO_NUM_35
#define TEST_ROTARY_DT GPIO_NUM_32
#define TEST_ROTARY_ALPS_A GPIO_NUM_33
#define TEST_ROTARY_ALPS_B GPIO_NUM_25

// Analog multiplexer pins for testing
#define TEST_AMTXER_SEL1 GPIO_NUM_5
#define TEST_AMTXER_SEL2 GPIO_NUM_18
#define TEST_AMTXER_SEL3 GPIO_NUM_19
#define TEST_AMTXER_IN1 GPIO_NUM_4
#define TEST_AMTXER_IN2 GPIO_NUM_16

// Analog clutch paddles for testing
#define TEST_ANALOG_PIN1 GPIO_NUM_36
#define TEST_ANALOG_PIN2 GPIO_NUM_39

// Battery monitor pins for testing
#define TEST_BATTERY_READ_ENABLE GPIO_NUM_17
#define TEST_BATTERY_READ GPIO_NUM_15

// Latch circuit pins for testing
#define TEST_LATCH_PIN GPIO_NUM_23

// PISO shift registers testing
#define TEST_SR_SERIAL GPIO_NUM_36
#define TEST_SR_LOAD GPIO_NUM_32
#define TEST_SR_NEXT GPIO_NUM_33

// Wake up
#define TEST_POWER_PIN TEST_ROTARY_SW

// Simple shift light UI
#define TEST_SIMPLE_SHIFT_LIGHT_PIN GPIO_NUM_33

// LED strip
#define TEST_D_OUT GPIO_NUM_21
#define TEST_LEVEL_SHIFTER true

// -------------------------------------------------------------
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
// -------------------------------------------------------------

// Button matrix pins for testing
#define TEST_BTNMTX_ROW1 GPIO_NUM_7
#define TEST_BTNMTX_ROW2 GPIO_NUM_15
#define TEST_BTNMTX_COL1 GPIO_NUM_4
#define TEST_BTNMTX_COL2 GPIO_NUM_5
#define TEST_BTNMTX_COL3 GPIO_NUM_6

// Relative rotary encoder pins for testing
#define TEST_ROTARY_SW GPIO_NUM_10
#define TEST_ROTARY_CLK GPIO_NUM_12
#define TEST_ROTARY_DT GPIO_NUM_11
#define TEST_ROTARY_ALPS_A GPIO_NUM_1
#define TEST_ROTARY_ALPS_B GPIO_NUM_38

// Analog multiplexer pins for testing
#define TEST_AMTXER_SEL1 GPIO_NUM_21
#define TEST_AMTXER_SEL2 GPIO_NUM_47
#define TEST_AMTXER_SEL3 GPIO_NUM_48
#define TEST_AMTXER_IN1 GPIO_NUM_16
#define TEST_AMTXER_IN2 GPIO_NUM_17

// Analog clutch paddles for testing
#define TEST_ANALOG_PIN1 GPIO_NUM_13
#define TEST_ANALOG_PIN2 GPIO_NUM_14

// Battery monitor pins for testing
#define TEST_BATTERY_READ_ENABLE GPIO_NUM_42
#define TEST_BATTERY_READ GPIO_NUM_2

// Latch circuit pins for testing
#define TEST_LATCH_PIN GPIO_NUM_18

// PISO shift registers testing
#define TEST_SR_SERIAL GPIO_NUM_41
#define TEST_SR_LOAD GPIO_NUM_39
#define TEST_SR_NEXT GPIO_NUM_40

// Wake up
#define TEST_POWER_PIN TEST_ROTARY_SW

// Simple shift light UI
#define TEST_SIMPLE_SHIFT_LIGHT_PIN GPIO_NUM_40

// Secondary I2C bus
#define TEST_SECONDARY_SDA GPIO_NUM_21
#define TEST_SECONDARY_SCL GPIO_NUM_47

// LED strip
#define TEST_D_OUT GPIO_NUM_39
#define TEST_LEVEL_SHIFTER false

// -------------------------------------------------------------
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
// -------------------------------------------------------------

#warning The ESP32C3 board is not configured for most test

// Button matrix pins for testing
#define TEST_BTNMTX_ROW1 GPIO_NUM_NC
#define TEST_BTNMTX_ROW2 GPIO_NUM_NC
#define TEST_BTNMTX_COL1 GPIO_NUM_NC
#define TEST_BTNMTX_COL2 GPIO_NUM_NC
#define TEST_BTNMTX_COL3 GPIO_NUM_NC

// Relative rotary encoder pins for testing
#define TEST_ROTARY_SW GPIO_NUM_NC
#define TEST_ROTARY_CLK GPIO_NUM_NC
#define TEST_ROTARY_DT GPIO_NUM_NC
#define TEST_ROTARY_ALPS_A GPIO_NUM_NC
#define TEST_ROTARY_ALPS_B GPIO_NUM_NC

// Analog multiplexer pins for testing
#define TEST_AMTXER_SEL1 GPIO_NUM_NC
#define TEST_AMTXER_SEL2 GPIO_NUM_NC
#define TEST_AMTXER_SEL3 GPIO_NUM_NC
#define TEST_AMTXER_IN1 GPIO_NUM_NC
#define TEST_AMTXER_IN2 GPIO_NUM_NC

// Analog clutch paddles for testing
#define TEST_ANALOG_PIN1 GPIO_NUM_NC
#define TEST_ANALOG_PIN2 GPIO_NUM_NC

// Battery monitor pins for testing
#define TEST_BATTERY_READ_ENABLE GPIO_NUM_NC
#define TEST_BATTERY_READ GPIO_NUM_NC

// Latch circuit pins for testing
#define TEST_LATCH_PIN GPIO_NUM_NC

// PISO shift registers testing
#define TEST_SR_SERIAL GPIO_NUM_NC
#define TEST_SR_LOAD GPIO_NUM_NC
#define TEST_SR_NEXT GPIO_NUM_NC

// Wake up
#define TEST_POWER_PIN GPIO_NUM_2

// Simple shift light UI
#define TEST_SIMPLE_SHIFT_LIGHT_PIN GPIO_NUM_NC

// -------------------------------------------------------------
#else
// -------------------------------------------------------------
#error Board not configured for testing in this project
#endif

// ------------------------------------------------------------
// ------------------------------------------------------------
// Globals
// ------------------------------------------------------------
// ------------------------------------------------------------

// Power latch
#define TEST_LATCH_MODE PowerLatchMode::POWER_OPEN_DRAIN
#define TEST_LATCH_DELAY 5000

// Shift registers
#define TEST_SR_BUTTONS_COUNT 17

// GPIO expanders
#define MCP23017_I2C_ADDR3 7
#define PCF8574_I2C_ADDR3 0

// ------------------------------------------------------------
// ------------------------------------------------------------
// Macros
// ------------------------------------------------------------
// ------------------------------------------------------------

#if !CD_CI
#include <HardwareSerial.h>

inline void debugPrintBool(uint64_t state, uint8_t bitCount = 0)
{
    int maxBitCount = (sizeof(uint64_t) * 8);
    if ((bitCount == 0) || (bitCount > maxBitCount))
        bitCount = maxBitCount;
    for (int i = (bitCount - 1); i >= 0; i--)
    {
        if ((1ULL << i) & state)
            Serial.print("1");
        else
            Serial.print("0");
    }
}
#endif

void setDebugInputNumbers(ButtonMatrix &instance)
{
    instance[TEST_BTNMTX_ROW1][TEST_BTNMTX_COL3] = 2;
    instance[TEST_BTNMTX_ROW2][TEST_BTNMTX_COL3] = 3;
    instance[TEST_BTNMTX_ROW2][TEST_BTNMTX_COL2] = 4;
    instance[TEST_BTNMTX_ROW1][TEST_BTNMTX_COL2] = 5;
    instance[TEST_BTNMTX_ROW1][TEST_BTNMTX_COL1] = 6;
    instance[TEST_BTNMTX_ROW2][TEST_BTNMTX_COL1] = 7;
}

OutputGPIOCollection getDebugMuxSelectors()
{
    return {TEST_AMTXER_SEL1, TEST_AMTXER_SEL2, TEST_AMTXER_SEL3};
}

void setDebugInputNumbers(AnalogMultiplexerGroup<Mux8Pin> &instance)
{
    AnalogMultiplexerChip8 chip1(TEST_AMTXER_IN1), chip2(TEST_AMTXER_IN2);
    chip1[Mux8Pin::A0] = 20;
    chip1[Mux8Pin::A7] = 21;
    chip2[Mux8Pin::A3] = 22;
    chip2[Mux8Pin::A5] = 23;
    instance = {chip1, chip2};
}

void setDebugInputNumbers(ShiftRegisterChain &instance, InputNumber &SER)
{
    ShiftRegisterChip chip1, chip2;
    chip1[SR8Pin::E] = 2;
    chip1[SR8Pin::B] = 4;
    chip2[SR8Pin::H] = 3;
    chip2[SR8Pin::C] = 5;
    SER = 6;
    instance = {chip1, chip2};
}

void setDebugInputNumbers(MCP23017Expander &instance)
{
    instance[MCP23017Pin::GPA0] = 10;
    instance[MCP23017Pin::GPA1] = 11;
    instance[MCP23017Pin::GPA2] = 12;
    instance[MCP23017Pin::GPA3] = 13;
    instance[MCP23017Pin::GPA4] = 14;
    instance[MCP23017Pin::GPA5] = 15;
    instance[MCP23017Pin::GPA6] = 16;
    instance[MCP23017Pin::GPA7] = 17;
    instance[MCP23017Pin::GPB0] = 20;
    instance[MCP23017Pin::GPB1] = 21;
    instance[MCP23017Pin::GPB2] = 22;
    instance[MCP23017Pin::GPB3] = 23;
    instance[MCP23017Pin::GPB4] = 24;
    instance[MCP23017Pin::GPB5] = 25;
    instance[MCP23017Pin::GPB6] = 26;
    instance[MCP23017Pin::GPB7] = 27;
}

void setDebugInputNumbers(PCF8574Expander &instance)
{
    instance[PCF8574Pin::P0] = 30;
    instance[PCF8574Pin::P1] = 31;
    instance[PCF8574Pin::P2] = 32;
    instance[PCF8574Pin::P3] = 33;
    instance[PCF8574Pin::P4] = 34;
    instance[PCF8574Pin::P5] = 35;
    instance[PCF8574Pin::P6] = 36;
    instance[PCF8574Pin::P7] = 37;
}

/// @endcond