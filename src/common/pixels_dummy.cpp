/**
 * @file Pixels_dummy.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-12-13
 * @brief Dummy implementation of the `pixels` namespace
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.h"
#include "SimWheelTypes.h"

// ----------------------------------------------------------------------------
// Configuration
// ----------------------------------------------------------------------------

void pixels::configure(
    pixelGroup_t group,
    gpio_num_t dataPin,
    uint8_t pixelCount,
    bool useLevelShift,
    pixel_driver_t pixelType,
    pixel_format_t pixelFormat) {}

uint8_t pixels::getPixelCount(pixelGroup_t group)
{
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
    uint8_t blue) {}

void pixels::reset() {}

void pixels::show() {}
