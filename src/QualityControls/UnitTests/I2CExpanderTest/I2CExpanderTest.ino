/**
 * @file I2CExpanderTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-03-02
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <HardwareSerial.h>
#include "debugUtils.h"
#include "i2cTools.h"
#include "I2CExpanderInput.h"
#include "PolledInput.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

I2CButtonsInput *chain;
inputBitmap_t state = 0ULL;

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    Serial.println("-- READY --");

    uint8_t pcf8574FullAddress, mcp23017FullAddress;
    i2c::require(1);
    Serial.println("Auto-detecting PCF8574 address");
    bool auto_detect_success = I2CInput::hardwareAddr2FullAddress(
        PCF8574_I2C_ADDR3,
        false,
        pcf8574FullAddress);
    if (auto_detect_success)
    {
        Serial.printf("Address is %x (hexadecimal)\n", pcf8574FullAddress);
        Serial.println("Auto-detecting MCP23017 address");
        auto_detect_success = I2CInput::hardwareAddr2FullAddress(
            MCP23017_I2C_ADDR3,
            false,
            mcp23017FullAddress);
    }
    if (auto_detect_success)
        Serial.printf("Address is %x (hexadecimal)\n", mcp23017FullAddress);
    else
    {
        Serial.println("ERROR: unable to auto-detect hardware addresses of I2C devices");
        Serial.println("Test failed.");
        for (;;)
            ;
    }

    I2CButtonsInput *pcf8574;
    pcf8574 = new PCF8574ButtonsInput(pcf8574FullAddress);
    setDebugInputNumbers(*(PCF8574ButtonsInput *)pcf8574);
    chain = new MCP23017ButtonsInput(
        mcp23017FullAddress,
        false,
        pcf8574);
    setDebugInputNumbers(*(MCP23017ButtonsInput *)chain);

    inputBitmap_t mask = DigitalPolledInput::getChainMask(chain);
    Serial.println("MASK:");
    debugPrintBool(mask);
    Serial.println("");
    Serial.println("-- GO --");
}

void loop()
{
    inputBitmap_t newState = DigitalPolledInput::readInChain(state, chain);
    if (state != newState)
    {
        state = newState;
        debugPrintBool(state);
        Serial.println("");
    }
    vTaskDelay(DEBOUNCE_TICKS*2);
}