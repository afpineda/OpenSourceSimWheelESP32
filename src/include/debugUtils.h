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
#define TEST_ROTARY_CLK GPIO_NUM_39
#define TEST_ROTARY_DT GPIO_NUM_34
#define TEST_ROTARY_SW GPIO_NUM_35

// Button matrix pins for testing
#define TEST_BTNMTX_ROW1 GPIO_NUM_25
#define TEST_BTNMTX_ROW2 GPIO_NUM_33
#define TEST_BTNMTX_ROW3 GPIO_NUM_32
#define TEST_BTNMTX_COL1 GPIO_NUM_26
#define TEST_BTNMTX_COL2 GPIO_NUM_27
static const gpio_num_t mtxSelectors[] = {TEST_BTNMTX_ROW1,TEST_BTNMTX_ROW2,TEST_BTNMTX_ROW3};
static const gpio_num_t mtxInputs[] = {TEST_BTNMTX_COL1,TEST_BTNMTX_COL2};

// Other pins for testing
#define TEST_DIGITAL_PIN GPIO_NUM_35
#define TEST_POWER_PIN GPIO_NUM_0

// Battery monitor pins for testing
#define TEST_BATTERY_READ_ENABLE GPIO_NUM_12
#define TEST_BATTERY_READ GPIO_NUM_13
// #define TEST_BATTERY_READ_ENABLE GPIO_NUM_12
// #define TEST_BATTERY_READ GPIO_NUM_2

// Latch circuit pins for testing
#define TEST_LATCH_PIN GPIO_NUM_12
#define TEST_LATCH_MODE POWER_OPEN_DRAIN
#define TEST_LATCH_DELAY 3000

#endif