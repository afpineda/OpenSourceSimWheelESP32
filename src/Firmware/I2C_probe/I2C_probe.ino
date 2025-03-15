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
#include "HAL.hpp"

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
// Auxiliary
//------------------------------------------------------------------

void dump_results(std::vector<uint8_t> &addressList)
{
    size_t count = addressList.size();
    Serial.printf("Auto-discovery finished. %d device(s) found:\n", count);
    for (int idx = 0; idx < count; idx++)
    {
        uint8_t addr = addressList.at(idx);
        Serial.printf("- Device found at address %x (hexadecimal), %d (decimal)\n",
                      addr,
                      addr);
        Serial.printf("  - Hardware address (3 bits) is %d\n",
                      (addr & 0b00000111));
        Serial.printf("  - Factory address (7 bits) is %x (hexadecimal), %d (decimal)\n",
                      (addr & 0b11111000), (addr & 0b11111000));
    }
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    Serial.println("=================================");
    Serial.println(" I2C slave device auto-discovery");
    Serial.println("=================================");
    Serial.println("");

    Serial.printf("SDA = #%d. SCL = #%d. Please, wait ...\n\n", SDA, SCL);
    std::vector<uint8_t> addressList;
    internals::hal::i2c::probe(addressList);
    dump_results(addressList);
    Serial.println("");
    Serial.println("");

#if defined(SECONDARY_SDA) && defined(SECONDARY_SCL)
    Serial.printf("SDA = #%d. SCL = #%d. Please, wait ...\n\n", SECONDARY_SDA, SECONDARY_SCL);
    internals::hal::i2c::initialize(
        SECONDARY_SDA,
        SECONDARY_SCL,
        I2CBus::SECONDARY);
    addressList.clear();
    internals::hal::i2c::probe(addressList, true);
    dump_results(addressList);
    Serial.println("");
    Serial.println("");
#endif

    Serial.println("Done. Reset to repeat auto-discovery.");
}

void loop()
{
}