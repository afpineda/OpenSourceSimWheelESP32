/**
 * @file debugUtils.h
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Debug and testing utilities
 *
 * @copyright Licensed under the EUPL
 *
 */

#ifndef __DEBUGUTILS_H__
#define __DEBUGUTILS_H__

#include <Arduino.h>
#include "SimWheelTypes.h"

void debugPrintBool(inputBitmap_t state, uint8_t bitsCount = 0);
void printTestHeader(int index);

// Relative rotary enconder pins for testing
#define TEST_ROTARY_SW GPIO_NUM_34
#define TEST_ROTARY_CLK GPIO_NUM_35
#define TEST_ROTARY_DT GPIO_NUM_32
#define TEST_ROTARY_ALPS_A GPIO_NUM_33
#define TEST_ROTARY_ALPS_B GPIO_NUM_25

// Button matrix pins for testing
#define TEST_BTNMTX_ROW1 GPIO_NUM_12
#define TEST_BTNMTX_ROW2 GPIO_NUM_13
#define TEST_BTNMTX_COL1 GPIO_NUM_26
#define TEST_BTNMTX_COL2 GPIO_NUM_27
#define TEST_BTNMTX_COL3 GPIO_NUM_14

static const gpio_num_array_t mtxSelectors = {TEST_BTNMTX_ROW1, TEST_BTNMTX_ROW2};
static const gpio_num_array_t mtxInputs= {TEST_BTNMTX_COL1, TEST_BTNMTX_COL2, TEST_BTNMTX_COL3};

void setDebugInputNumbers(ButtonMatrixInputSpec &instance);

// Analog multiplexer pins for testing
#define TEST_AMTXER_SEL1 GPIO_NUM_5
#define TEST_AMTXER_SEL2 GPIO_NUM_18
#define TEST_AMTXER_SEL3 GPIO_NUM_19
#define TEST_AMTXER_IN1 GPIO_NUM_4
#define TEST_AMTXER_IN2 GPIO_NUM_16

static const gpio_num_array_t amtxerSelectors = {TEST_AMTXER_SEL1, TEST_AMTXER_SEL2, TEST_AMTXER_SEL3};
static const gpio_num_array_t amtxerInputs = {TEST_AMTXER_IN1, TEST_AMTXER_IN2};

void setDebugInputNumbers(Multiplexers8InputSpec &instance);

// Other pins for testing
#define TEST_ANALOG_PIN1 GPIO_NUM_36
#define TEST_ANALOG_PIN2 GPIO_NUM_39

// Battery monitor pins for testing
#define TEST_BATTERY_READ_ENABLE GPIO_NUM_17
#define TEST_BATTERY_READ GPIO_NUM_15

// Latch circuit pins for testing
#define TEST_LATCH_PIN GPIO_NUM_23
#define TEST_LATCH_MODE POWER_OPEN_DRAIN
#define TEST_LATCH_DELAY pdMS_TO_TICKS(5000)

// PISO shift registers testing
#define TEST_SR_SERIAL GPIO_NUM_36
#define TEST_SR_LOAD GPIO_NUM_32
#define TEST_SR_NEXT GPIO_NUM_33

// static const inputNumber_t srNumbers[] = {
//     63, 62, 61, 2, 60, 59, 4, 58,
//     3, 57, 56, 55, 54, 5, 53, 52,
//     6};

#define TEST_SR_BUTTONS_COUNT 17

void setDebugInputNumbers(ShiftRegisters8InputSpec &instance);

// GPIO expander testing

// #define MCP23017_I2C_ADDR7 0b00100000
// #define PCF8574_I2C_ADDR7 0b00111001
#define MCP23017_I2C_ADDR3 0
#define PCF8574_I2C_ADDR3 1

void setDebugInputNumbers(MCP23017InputSpec &instance);
void setDebugInputNumbers(PCF8574InputSpec &instance);

// Deep sleep testing
#ifndef CONFIG_IDF_TARGET_ESP32C3
#define TEST_POWER_PIN TEST_ROTARY_SW
#else
#define TEST_POWER_PIN GPIO_NUM_2
#endif

#endif
