/**
 * @file I2CExpanderTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-03
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <HardwareSerial.h>
#include "debugUtils.h"
#include "I2CExpanderInput.h"
#include "PolledInput.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

I2CButtonsInput *chain;
inputNumber_t pcf8574BtnNumbers[] = {1, 2, 3, 4, 5, 6};
inputNumber_t mcp23017BtnNumbers[] = {9, 10, 11, 12, 13, 14};
#define EXPECTED_MASK 0b0111111001111110ULL

#define PCF8574_ADDR 0b0010000  // hardware address 0b000
#define MCP23017_ADDR 0b0010001 // hardware address 0b001

inputBitmap_t state = 0ULL;

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    Serial.println("-- READY --");

    I2CButtonsInput *pcf8574;
    pcf8574 = new PCF8574ButtonsInput(
        sizeof(pcf8574BtnNumbers) / sizeof(pcf8574BtnNumbers[0]),
        pcf8574BtnNumbers,
        PCF8574_ADDR);
    chain = new MCP23017ButtonsInput(
        sizeof(mcp23017BtnNumbers) / sizeof(mcp23017BtnNumbers[0]),
        mcp23017BtnNumbers,
        MCP23017_ADDR,
        false,
        pcf8574);

    inputBitmap_t mask = DigitalPolledInput::getChainMask(chain);

    if (mask != EXPECTED_MASK)
    {
        Serial.println("ERROR. UNEXPECTED MASK:");
        debugPrintBool(mask);
        Serial.println("");
    }
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
    vTaskDelay(DEBOUNCE_TICKS);
}