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

#include "HAL.hpp"
#include "Testing.hpp"

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
    debugPrintf("Auto-discovery finished. %d device(s) found:\n", count);
    for (int idx = 0; idx < count; idx++)
    {
        uint8_t addr = addressList.at(idx);
        debugPrintf(
            "- Device found at address %x (hexadecimal), %d (decimal)\n",
            addr,
            addr);
        debugPrintf(
            "  - Hardware address (3 bits) is %d\n",
            (addr & 0b00000111));
        debugPrintf(
            "  - Factory address (7 bits) is %x (hexadecimal), %d (decimal)\n",
            (addr & 0b11111000), (addr & 0b11111000));
    }
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    debugPrintBegin();
    debugPrintf("=================================\n");
    debugPrintf(" I2C slave device auto-discovery\n");
    debugPrintf("=================================\n\n");

#if defined(SECONDARY_SDA) && defined(SECONDARY_SCL)
    internals::hal::i2c::initialize(
        SECONDARY_SDA,
        SECONDARY_SCL,
        I2CBus::SECONDARY);
#endif
}

void loop()
{
    std::vector<uint8_t> addressList;
    debugPrintf("SDA = #%d. SCL = #%d. Please, wait ...\n\n", SDA, SCL);
    internals::hal::i2c::probe(addressList);
    dump_results(addressList);
    debugPrintf("\n\n");

#if defined(SECONDARY_SDA) && defined(SECONDARY_SCL)
    debugPrintf(
        "SDA = #%d. SCL = #%d. Please, wait ...\n\n",
        SECONDARY_SDA,
        SECONDARY_SCL);
    addressList.clear();
    internals::hal::i2c::probe(addressList, I2CBus::SECONDARY);
    dump_results(addressList);
    debugPrintf("\n\n");
#endif

    debugPrintf("Done. Repeating autodiscovery in 30 seconds...\n\n\n");

    vTaskDelay(pdMS_TO_TICKS(30 * 1000));
}