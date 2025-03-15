/**
 * @file PCF8574LedDriverTest.ino
 *
 * @date 2024-03-07
 * @brief Unit Test
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "Testing.hpp"
#include "OutputHardware.hpp"
#include "HAL.hpp"

#include <HardwareSerial.h>

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

PCF8574LedDriver *driver;
#define DEFAULT_DELAY 2000

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    Serial.println("--READY--");

    // Device discovery in the secondary bus.
    // Just to make sure we are using a correct I2C address.
    internals::hal::i2c::initialize(
        TEST_SECONDARY_SDA,
        TEST_SECONDARY_SCL,
        I2CBus::SECONDARY);
    Serial.printf(
        "SDA = #%d. SCL = #%d. Auto-discovery in progress ...\n\n",
        TEST_SECONDARY_SDA,
        TEST_SECONDARY_SCL);
    std::vector<uint8_t> addressList;
    internals::hal::i2c::probe(addressList, I2CBus::SECONDARY);
    size_t count = addressList.size();
    Serial.printf("Auto-discovery finished. %d device(s) found:\n", count);
    for (int idx = 0; idx < count; idx++)
    {
        uint8_t addr = addressList.at(idx);
        Serial.printf("- Device found at address %x (hexadecimal), %d (decimal)\n", addr, addr);
    }

    // Configure I2C address
    uint8_t hwAddress = 2;
    uint8_t factoryAddress = 0x38;
    Serial.printf("Using device address %x (hexadecimal)\n", factoryAddress | hwAddress);

    // Initialize
    driver = new PCF8574LedDriver(I2CBus::SECONDARY, factoryAddress | hwAddress);
    Serial.println("--GO--");
}


void loop()
{
    Serial.println("All ON");
    driver->setState(0b11111111);
    driver->show();
    DELAY_MS(DEFAULT_DELAY);

    Serial.println("All OFF");
    driver->swap();
    driver->show();
    DELAY_MS(DEFAULT_DELAY);

    Serial.println("Shift right");
    driver->setState(0b1);
    driver->show();
    DELAY_MS(DEFAULT_DELAY);
    for (int i=0; i<7; i++)
    {
        driver->shiftRight();
        driver->show();
        DELAY_MS(DEFAULT_DELAY);
    }

    Serial.println("Shift left");
    driver->setState(0b10000000);
    driver->show();
    DELAY_MS(DEFAULT_DELAY);
    for (int i=0; i<7; i++)
    {
        driver->shiftLeft();
        driver->show();
        DELAY_MS(DEFAULT_DELAY);
    }

    Serial.println("Extremes ON");
    driver->setState(0);
    driver->setLed(0,true);
    driver->setLed(7,true);
    driver->show();
    DELAY_MS(DEFAULT_DELAY);
}