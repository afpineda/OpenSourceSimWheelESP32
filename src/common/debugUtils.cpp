/**
 * @file debugUtils.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Testing and debugging utilities
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "debugUtils.h"
#include <Arduino.h>

void debugPrintBool(inputBitmap_t state, uint8_t bitCount)
{
    int maxBitCount = (sizeof(inputBitmap_t) * 8);
    if ((bitCount == 0) || (bitCount > maxBitCount))
        bitCount = maxBitCount;
    for (int i = (bitCount - 1); i >= 0; i--)
    {
        if (BITMAP(i) & state)
            Serial.print("1");
        else
            Serial.print("0");
    }
}

void printTestHeader(int index)
{
    Serial.print("**TEST #");
    Serial.println(index);
}

void setDebugInputNumbers(ShiftRegisters8InputSpec &instance)
{
    instance.inputNumber(0,sr8_pin_t::E,2);
    instance.inputNumber(0,sr8_pin_t::B,4);
    instance.inputNumber(1,sr8_pin_t::H,3);
    instance.inputNumber(1,sr8_pin_t::C,5);
    instance.inputNumber(1,sr8_pin_t::SER,6);
}

void setDebugInputNumbers(MCP23017InputSpec &instance)
{
    instance.inputNumber(MCP23017_pin_t::GPA0,10);
    instance.inputNumber(MCP23017_pin_t::GPA1,11);
    instance.inputNumber(MCP23017_pin_t::GPA2,12);
    instance.inputNumber(MCP23017_pin_t::GPA3,13);
    instance.inputNumber(MCP23017_pin_t::GPA4,14);
    instance.inputNumber(MCP23017_pin_t::GPA5,15);
    instance.inputNumber(MCP23017_pin_t::GPA6,16);
    instance.inputNumber(MCP23017_pin_t::GPA7,17);
    instance.inputNumber(MCP23017_pin_t::GPB0,20);
    instance.inputNumber(MCP23017_pin_t::GPB1,21);
    instance.inputNumber(MCP23017_pin_t::GPB2,22);
    instance.inputNumber(MCP23017_pin_t::GPB3,23);
    instance.inputNumber(MCP23017_pin_t::GPB4,24);
    instance.inputNumber(MCP23017_pin_t::GPB5,25);
    instance.inputNumber(MCP23017_pin_t::GPB6,26);
    instance.inputNumber(MCP23017_pin_t::GPB7,27);
}

void setDebugInputNumbers(PCF8574InputSpec &instance)
{
    instance.inputNumber(PCF8574_pin_t::P0,30);
    instance.inputNumber(PCF8574_pin_t::P1,31);
    instance.inputNumber(PCF8574_pin_t::P2,32);
    instance.inputNumber(PCF8574_pin_t::P3,33);
    instance.inputNumber(PCF8574_pin_t::P4,34);
    instance.inputNumber(PCF8574_pin_t::P5,35);
    instance.inputNumber(PCF8574_pin_t::P6,36);
    instance.inputNumber(PCF8574_pin_t::P7,37);
}

void setDebugInputNumbers(Multiplexers8InputSpec &instance)
{
    instance.inputNumber(TEST_AMTXER_IN1,mux8_pin_t::A0,20);
    instance.inputNumber(TEST_AMTXER_IN1,mux8_pin_t::A7,21);
    instance.inputNumber(TEST_AMTXER_IN2,mux8_pin_t::A3,22);
    instance.inputNumber(TEST_AMTXER_IN2,mux8_pin_t::A5,23);
}


void setDebugInputNumbers(ButtonMatrixInputSpec &instance)
{
    instance.inputNumber(TEST_BTNMTX_ROW1,TEST_BTNMTX_COL3,2);
    instance.inputNumber(TEST_BTNMTX_ROW2,TEST_BTNMTX_COL3,3);
    instance.inputNumber(TEST_BTNMTX_ROW2,TEST_BTNMTX_COL2,4);
    instance.inputNumber(TEST_BTNMTX_ROW1,TEST_BTNMTX_COL2,5);
    instance.inputNumber(TEST_BTNMTX_ROW1,TEST_BTNMTX_COL1,6);
    instance.inputNumber(TEST_BTNMTX_ROW2,TEST_BTNMTX_COL1,7);
}