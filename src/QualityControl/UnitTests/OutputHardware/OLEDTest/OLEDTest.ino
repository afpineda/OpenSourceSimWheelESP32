/**
 * @file OLEDTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2026-03-02
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "OutputHardware.hpp"
#include "Testing.hpp"
#include "esp32-hal.h"
#include "HAL.hpp"

SSD1306 *hw = nullptr;

uint8_t test_buffer[] = {0xFF, 0xFF, 0xFF, 0xFF};

void setup()
{
    debugPrintBegin();
    debugPrintf("SCL: %u, SDA: %u\n", SCL, SDA);
    hw = new SSD1306(I2CBus::PRIMARY, SSD1306::_address7bits);
    if (!hw->available())
        debugPrintf("Device not found\n");
    debugPrintf("--GO--\n");
    hw->clear(true);
    // hw->write_buffer(test_buffer,sizeof(test_buffer));
}

uint8_t contrast = 0xFF;

void loop()
{
    // DELAY_MS(1000);
    // hw->inverse_display(true);
    // DELAY_MS(1000);
    // hw->inverse_display(false);

    hw->contrast(contrast--);
    DELAY_MS(50);

    // hw->turn(false);
    // DELAY_MS(1000);
    // hw->turn(true);
    // DELAY_MS(1000);
}