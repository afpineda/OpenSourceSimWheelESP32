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

// bool hardwareAddr2FullAddress(
//     uint8_t address3bits,
//     i2c_port_t bus,
//     uint8_t &address7bits)
// {
//     address3bits = address3bits & 0b00000111;
//     Serial.printf("Hardware address: %x\n",address3bits);
//     uint8_t count = 0;
//     for (uint8_t other4bits = 0; other4bits < 16; other4bits++)
//     {
//         uint8_t tryAddress = (other4bits << 3) | address3bits;
//         Serial.printf("Trying: %x\n",tryAddress);
//         if (I2CInput::probe(tryAddress, bus)) {
//             Serial.println("*Match*");
//             address7bits = tryAddress;
//             count++;
//         }
//     }
//     return (count==1);
// }

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    Serial.println("-- READY --");

    uint8_t pcf8574FullAddress, mcp23017FullAddress;
    I2CInput::initializePrimaryBus();
    Serial.println("Auto-detecting PCF8574 address");
    bool auto_detect_success = I2CInput::hardwareAddr2FullAddress(
        PCF8574_I2C_ADDR3,
        I2CInput::getBusDriver(),
        pcf8574FullAddress);
    if (auto_detect_success)
    {
        Serial.printf("Address is %x (hexadecimal)\n", pcf8574FullAddress);
        Serial.println("Auto-detecting MCP23017 address");
        auto_detect_success = I2CInput::hardwareAddr2FullAddress(
            MCP23017_I2C_ADDR3,
            I2CInput::getBusDriver(),
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