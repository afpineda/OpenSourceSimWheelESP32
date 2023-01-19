/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Debug and testing utilities
 * 
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 * 
 */

#ifndef __DEBUGUTILS_H__
#define __DEBUGUTILS_H__

#include <Arduino.h>
#include "SimWheelTypes.h"

void debugPrintBool(inputBitmap_t state);
void printTestHeader(int index);
void serialPrintf(const char *fmt, ...);

// Relative rotary enconder pins for testing
#define TEST_ROTARY_CLK GPIO_NUM_36
#define TEST_ROTARY_DT GPIO_NUM_39
#define TEST_ROTARY_SW GPIO_NUM_34
#define TEST_ROTARY_ALPS_A GPIO_NUM_35
#define TEST_ROTARY_ALPS_B GPIO_NUM_14

// Button matrix pins for testing
#define TEST_BTNMTX_ROW1 GPIO_NUM_26
#define TEST_BTNMTX_ROW2 GPIO_NUM_27
#define TEST_BTNMTX_COL1 GPIO_NUM_32
#define TEST_BTNMTX_COL2 GPIO_NUM_33
#define TEST_BTNMTX_COL3 GPIO_NUM_25

static const gpio_num_t mtxSelectors[] = {TEST_BTNMTX_ROW1,TEST_BTNMTX_ROW2};
static const gpio_num_t mtxInputs[] = {TEST_BTNMTX_COL1,TEST_BTNMTX_COL2,TEST_BTNMTX_COL3};
static inputNumber_t mtxNumbers[] = {6,7,5,4,2,3};

// Analog multiplexer pins for testing
#define TEST_AMTXER_SEL1 GPIO_NUM_19
#define TEST_AMTXER_IN1 GPIO_NUM_18
#define TEST_AMTXER_IN2 GPIO_NUM_5

static const gpio_num_t amtxerSelectors[] = {TEST_AMTXER_SEL1};
static const gpio_num_t amtxerInputs[] = {TEST_AMTXER_IN1,TEST_AMTXER_IN2};
static inputNumber_t amtxerNumbers[] = {20,21,22,23};

// Other pins for testing
#define TEST_ANALOG_PIN1 GPIO_NUM_12
#define TEST_ANALOG_PIN2 GPIO_NUM_13

// Battery monitor pins for testing
#define TEST_BATTERY_READ_ENABLE GPIO_NUM_17
#define TEST_BATTERY_READ GPIO_NUM_15

// Latch circuit pins for testing
#define TEST_LATCH_PIN GPIO_NUM_23
#define TEST_LATCH_MODE POWER_OPEN_DRAIN
#define TEST_LATCH_DELAY pdMS_TO_TICKS(5000)

#endif