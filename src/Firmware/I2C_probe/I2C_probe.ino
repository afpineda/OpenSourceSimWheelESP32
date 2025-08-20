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
#include <HardwareSerial.h>
#include "USBCDC.h"

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
    Serial0.printf("Auto-discovery finished. %d device(s) found:\n", count);
#ifdef USB_SERIAL_IS_DEFINED
    USBSerial.printf("Auto-discovery finished. %d device(s) found:\n", count);
#endif
    for (int idx = 0; idx < count; idx++)
    {
        uint8_t addr = addressList.at(idx);
        Serial0.printf("- Device found at address %x (hexadecimal), %d (decimal)\n",
                      addr,
                      addr);
        Serial0.printf("  - Hardware address (3 bits) is %d\n",
                      (addr & 0b00000111));
        Serial0.printf("  - Factory address (7 bits) is %x (hexadecimal), %d (decimal)\n",
                      (addr & 0b11111000), (addr & 0b11111000));
#ifdef USB_SERIAL_IS_DEFINED
        USBSerial.printf("- Device found at address %x (hexadecimal), %d (decimal)\n",
                         addr,
                         addr);
        USBSerial.printf("  - Hardware address (3 bits) is %d\n",
                         (addr & 0b00000111));
        USBSerial.printf("  - Factory address (7 bits) is %x (hexadecimal), %d (decimal)\n",
                         (addr & 0b11111000), (addr & 0b11111000));
#endif
    }
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial0.begin(115200);
    Serial0.println("=================================");
    Serial0.println(" I2C slave device auto-discovery");
    Serial0.println("=================================");
    Serial0.println("");

#ifdef USB_SERIAL_IS_DEFINED
    USBSerial.begin(115200);
    USBSerial.println("=================================");
    USBSerial.println(" I2C slave device auto-discovery");
    USBSerial.println("=================================");
    USBSerial.println("");
#endif

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
    Serial0.printf("SDA = #%d. SCL = #%d. Please, wait ...\n\n", SDA, SCL);
#ifdef USB_SERIAL_IS_DEFINED
    USBSerial.printf("SDA = #%d. SCL = #%d. Please, wait ...\n\n", SDA, SCL);
#endif
    internals::hal::i2c::probe(addressList);
    dump_results(addressList);
    Serial0.println("");
    Serial0.println("");

#if defined(SECONDARY_SDA) && defined(SECONDARY_SCL)
    Serial0.printf("SDA = #%d. SCL = #%d. Please, wait ...\n\n", SECONDARY_SDA, SECONDARY_SCL);
#ifdef USB_SERIAL_IS_DEFINED
    USBSerial.printf("SDA = #%d. SCL = #%d. Please, wait ...\n\n", SECONDARY_SDA, SECONDARY_SCL);
#endif
    addressList.clear();
    internals::hal::i2c::probe(addressList, I2CBus::SECONDARY);
    dump_results(addressList);
    Serial0.println("");
    Serial0.println("");
#ifdef USB_SERIAL_IS_DEFINED
    USBSerial.println("");
    USBSerial.println("");
#endif

#endif

    Serial0.println("Done. Repeating autodiscovery in 30 seconds...");
    Serial0.println("");
    Serial0.println("");
#ifdef USB_SERIAL_IS_DEFINED
    USBSerial.println("Done. Repeating autodiscovery in 30 seconds...");
    USBSerial.println("");
    USBSerial.println("");
#endif

    vTaskDelay(pdMS_TO_TICKS(30 * 1000));
}