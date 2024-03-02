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

    // Serial.println("  ** I2C auto-discovery start");
    // I2CInput::initializePrimaryBus();
    // for (int addr=0; addr<127; addr++) {
    //     if (I2CInput::probe(addr,0)) {
    //          Serial.printf("  I2C device found at address 0x%x (%d)\n",addr,addr);
    //     }
    // }
    // Serial.println("  ** I2C auto-discovery end");

    I2CButtonsInput *pcf8574;
    pcf8574 = new PCF8574ButtonsInput(
        pcf8574Numbers,
        PCF8574_I2C_ADDR7);
    chain = new MCP23017ButtonsInput(
        mcp23017Numbers,
        MCP23017_I2C_ADDR7,
        false,
        pcf8574);

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
    vTaskDelay(DEBOUNCE_TICKS);
}