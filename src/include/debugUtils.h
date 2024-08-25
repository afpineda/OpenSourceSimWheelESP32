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

#ifndef __DEBUG_UTILS_H__
#define __DEBUG_UTILS_H__

#include "SimWheelTypes.h"
#include "esp32-hal.h"

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

// Button matrix
static const gpio_num_array_t mtxSelectors = {TEST_BTNMTX_ROW1, TEST_BTNMTX_ROW2};
static const gpio_num_array_t mtxInputs = {TEST_BTNMTX_COL1, TEST_BTNMTX_COL2, TEST_BTNMTX_COL3};

// Analog multiplexers
static const gpio_num_array_t amtxerSelectors = {TEST_AMTXER_SEL1, TEST_AMTXER_SEL2, TEST_AMTXER_SEL3};
static const gpio_num_array_t amtxerInputs = {TEST_AMTXER_IN1, TEST_AMTXER_IN2};

// Power latch
#define TEST_LATCH_MODE POWER_OPEN_DRAIN
#define TEST_LATCH_DELAY pdMS_TO_TICKS(5000)

// Shift registers
#define TEST_SR_BUTTONS_COUNT 17

// GPIO expanders
#define MCP23017_I2C_ADDR3 0
#define PCF8574_I2C_ADDR3 1

// ------------------------------------------------------------
// ------------------------------------------------------------
// Macros
// ------------------------------------------------------------
// ------------------------------------------------------------

void debugPrintBool(inputBitmap_t state, uint8_t bitsCount = 0);
void printTestHeader(int index);
void setDebugInputNumbers(ButtonMatrixInputSpec &instance);
void setDebugInputNumbers(Multiplexers8InputSpec &instance);
void setDebugInputNumbers(ShiftRegisters8InputSpec &instance);
void setDebugInputNumbers(MCP23017InputSpec &instance);
void setDebugInputNumbers(PCF8574InputSpec &instance);

#endif //__DEBUG_UTILS_H__
