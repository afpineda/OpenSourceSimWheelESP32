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
    pixel_format_t pixelFormat,
    uint8_t globalBrightness) {}

void pixels::shutdown() {}

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

void pixels::setAll(pixelGroup_t group,
                    uint8_t red,
                    uint8_t green,
                    uint8_t blue) {}

void pixels::shiftToNext(pixelGroup_t group) {}

void pixels::shiftToPrevious(pixelGroup_t group) {}

void pixels::reset() {}

void pixels::show() {}

// ----------------------------------------------------------------------------
// Notifications
// ----------------------------------------------------------------------------

void PixelControlNotification::onStart() {}

void PixelControlNotification::onBitePoint() {}

void PixelControlNotification::onConnected() {}

void PixelControlNotification::onBLEdiscovering() {}

void PixelControlNotification::onLowBattery() {}

void PixelControlNotification::pixelControl_OnStart() {}

void PixelControlNotification::pixelControl_OnBitePoint() {}

void PixelControlNotification::pixelControl_OnConnected() {}

void PixelControlNotification::pixelControl_OnBLEdiscovering() {}

void PixelControlNotification::pixelControl_OnLowBattery() {}
