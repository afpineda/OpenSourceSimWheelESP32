/**
 * @file I2C_probe.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-03-02
 * @brief Reveal address of slave devices in the I2C bus.
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <HardwareSerial.h>
#include "i2cTools.h"

//------------------------------------------------------------------
// GLOBALS
//------------------------------------------------------------------

// [EN] Uncomment the following lines to discover devices
//      in a secondary I2C bus.
//      Put the desired SDA and SCL pin numbers (or aliases)
//      to the right.

// #define SECONDARY_SDA
// #define SECONDARY_SCL

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    uint8_t count = 0;
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    Serial.println("=================================");
    Serial.println(" I2C slave device auto-discovery");
    Serial.println("=================================");
    Serial.println("");

    Serial.printf("SDA = #%d. SCL = #%d. Please, wait ...\n\n", SDA, SCL);
    std::vector<uint8_t> addressList;
    i2c::probe(addressList);
    count = addressList.size();
    Serial.printf("Auto-discovery finished. %d device(s) found:\n", count);
    for (int idx = 0; idx < count; idx++)
    {
        uint8_t addr = addressList.at(idx);
        Serial.printf("- Device found at address %x (hexadecimal), %d (decimal)\n", addr, addr);
    }
    Serial.println("");
    Serial.println("");

#if defined(SECONDARY_SDA) && defined(SECONDARY_SCL)
    Serial.printf("SDA = #%d. SCL = #%d. Please, wait ...\n\n", SECONDARY_SDA, SECONDARY_SCL);
    i2c::begin(
        SECONDARY_SDA,
        SECONDARY_SCL,
        true);
    addressList.clear();
    i2c::probe(addressList, true);
    count = addressList.size();
    Serial.printf("Auto-discovery finished. %d device(s) found:\n", count);
    for (int idx = 0; idx < count; idx++)
    {
        uint8_t addr = addressList.at(idx);
        Serial.printf("- Device found at address %x (hexadecimal), %d (decimal)\n", addr, addr);
    }
    Serial.println("");
    Serial.println("");
#endif

    Serial.println("Done. Reset to repeat auto-discovery.");
}

void loop()
{
}