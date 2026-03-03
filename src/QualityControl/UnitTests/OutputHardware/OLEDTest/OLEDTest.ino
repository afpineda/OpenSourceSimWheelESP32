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

OLED *hw = nullptr;

uint8_t test_buffer[] = {0xFF, 0xFF, 0xFF, 0xFF};

void setup()
{
    debugPrintBegin();
    debugPrintf("SCL: %u, SDA: %u\n", SCL, SDA);
    hw = new OLED(I2CBus::PRIMARY);
    if (!hw->available())
        debugPrintf("Device not found\n");
    debugPrintf("--GO--\n");

    uint8_t st;
    if (hw->read_status(st))
    {
        debugPrintf("Status %x: \n", st);
    }
    else
        debugPrintf("No status \n");

    hw->write_cmd(0x20, 0);      // horizontal addresing
    hw->write_cmd(0x21, 0, 127); // Set column range
    hw->write_cmd(0x22, 0, 7);   // Set page range
    hw->setPixel(0, 0, true);
    hw->setPixel(1, 1, true);
    hw->setPixel(2, 2, true);
    hw->setPixel(3, 3, true);
    hw->setPixel(0, 1, true);
    hw->setPixel(0, 2, true);
    hw->setPixel(0, 3, true);
    hw->write_gdd_ram(hw->_frame, 127);
}

uint8_t contrast = 0xFF;

void loop()
{
}