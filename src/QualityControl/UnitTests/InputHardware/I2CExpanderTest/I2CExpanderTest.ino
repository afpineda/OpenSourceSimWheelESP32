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

#include "Testing.hpp"
#include "InputHardware.hpp"
#include "HAL.hpp"

#include <HardwareSerial.h>

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

uint64_t state = 0ULL;
I2CInput *pcf8574;
I2CInput *mcp23017;

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    Serial.println("-- READY --");

    Serial.println("Auto-detecting I2C devices...\n");
    std::vector<uint8_t> fullAddressList;
    internals::hal::i2c::probe(fullAddressList);
    Serial.printf("%d device(s) found.\n", fullAddressList.size());

    uint8_t pcf8574FullAddress =
        internals::hal::i2c::findFullAddress(fullAddressList, PCF8574_I2C_ADDR3);
    if (pcf8574FullAddress < 128)
        Serial.printf("PCF8574 address is %x (hexadecimal)\n", pcf8574FullAddress);
    else
        Serial.printf("ERROR: PCF8574 address not found or not unique (%x)\n", pcf8574FullAddress);

    uint8_t mcp23017FullAddress =
        internals::hal::i2c::findFullAddress(fullAddressList, MCP23017_I2C_ADDR3);
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

    PCF8574Expander spec1;
    MCP23017Expander spec2;
    setDebugInputNumbers(spec1);
    setDebugInputNumbers(spec2);

    pcf8574 = new PCF8574ButtonsInput(
        spec1,
        pcf8574FullAddress);
    mcp23017 = new MCP23017ButtonsInput(
        spec2,
        mcp23017FullAddress);

    uint64_t mask = pcf8574->mask & mcp23017->mask;
    Serial.println("MASK:");
    debugPrintBool(mask);
    Serial.println("");
    Serial.println("-- GO --");
}

void loop()
{
    uint64_t newState = pcf8574->read(state) | mcp23017->read(state);
    if (state != newState)
    {
        state = newState;
        debugPrintBool(state);
        Serial.println("");
    }
    DELAY_MS(60);
}