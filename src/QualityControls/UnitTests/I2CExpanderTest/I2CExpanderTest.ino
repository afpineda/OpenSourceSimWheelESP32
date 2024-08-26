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

    Serial.println("Auto-detecting I2C devices...\n");
    std::vector<uint8_t> fullAddressList;
    i2c::probe(fullAddressList);
    Serial.printf("%d device(s) found.\n", fullAddressList.size());

    uint8_t pcf8574FullAddress = i2c::findFullAddress(fullAddressList, PCF8574_I2C_ADDR3);
    if (pcf8574FullAddress < 128)
        Serial.printf("PCF8574 address is %x (hexadecimal)\n", pcf8574FullAddress);
    else
        Serial.printf("ERROR: PCF8574 address not found or not unique (%x)\n", pcf8574FullAddress);

    uint8_t mcp23017FullAddress = i2c::findFullAddress(fullAddressList, MCP23017_I2C_ADDR3);
    if (mcp23017FullAddress < 128)
        Serial.printf("MCP23017 address is %x (hexadecimal)\n", mcp23017FullAddress);
    else
        Serial.printf("ERROR: MCP23017 address not found or not unique (%x)\n", mcp23017FullAddress);

    if ((pcf8574FullAddress > 127) || (mcp23017FullAddress > 127))
    {
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
    vTaskDelay(DEBOUNCE_TICKS * 2);
}