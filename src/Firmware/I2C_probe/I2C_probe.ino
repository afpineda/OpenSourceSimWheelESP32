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
#include "PolledInput.h"

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

    I2CInput::initializePrimaryBus();
    for (int addr = 0; addr < 128; addr++)
    {
        if (I2CInput::probe(addr, 0))
        {
            count++;
            Serial.printf("Device found at address %x (hexadecimal), %d (decimal))\n", addr, addr);
        }
    }
    Serial.println("");
    Serial.printf("Auto-discovery finished. %d devices found.", count);
    Serial.println("");
    Serial.println("Done. Reset to repeat auto-discovery.");
}

void loop()
{
}