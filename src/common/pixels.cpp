/**
 * @file Pixels.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-12-13
 * @brief Implementation of the `pixels` namespace
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.h"
#include "SimWheelTypes.h"
#include "LedStrip.h"

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

static LEDStrip *pixelData[3] = {nullptr};

// ----------------------------------------------------------------------------
// Configuration
// ----------------------------------------------------------------------------

void pixels::configure(
    pixelGroup_t group,
    gpio_num_t dataPin,
    uint8_t pixelCount,
    bool useLevelShift,
    pixel_driver_t pixelType,
    pixel_format_t pixelFormat)
{
    if (pixelData[group] != nullptr)
        delete pixelData[group];
    pixelData[group] = new LEDStrip(
        dataPin,
        pixelCount,
        useLevelShift,
        pixelType,
        pixelFormat);
    pixelData[group]->brightness(255);
}

uint8_t pixels::getPixelCount(pixelGroup_t group)
{
    if (pixelData[group] != nullptr)
        return pixelData[group]->getPixelCount();
    else
        return 0;
}

// ----------------------------------------------------------------------------
// Pixel control
// ----------------------------------------------------------------------------

void pixels::set(
    pixelGroup_t group,
    uint8_t pixelIndex,
    uint8_t red,
    uint8_t green,
    uint8_t blue)
{
    if (pixelData[group] != nullptr)
        pixelData[group]->pixelRGB(pixelIndex, red, green, blue);
}

void pixels::reset()
{
    for (int i = 0; i < 3; i++)
        if (pixelData[i] != nullptr)
        {
            pixelData[i]->pixelRangeRGB(0, 255, 0, 0, 0);
            pixelData[i]->show();
        }
}

void pixels::show()
{
    for (int i = 0; i < 3; i++)
        if (pixelData[i] != nullptr)
            pixelData[i]->show();
}
