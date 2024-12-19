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

LEDStrip *pixelData[3] = {nullptr};
SemaphoreHandle_t pixelLock = nullptr;
StaticSemaphore_t pixelLockBuffer;
#define WAIT_TICKS pdMS_TO_TICKS(150)

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
    uint8_t globalBrightness)
{
    if (pixelData[group] != nullptr)
        delete pixelData[group];
    if (pixelLock == nullptr)
    {
        // pixelLock = xSemaphoreCreateRecursiveMutexStatic(&pixelLockBuffer);
        pixelLock = xSemaphoreCreateRecursiveMutex();
        if (pixelLock == nullptr)
        {
            log_e("pixels::configure() unable to create mutex");
            abort();
        }
    }
    pixelData[group] = new LEDStrip(
        dataPin,
        pixelCount,
        useLevelShift,
        pixelType,
        pixelFormat);
    pixelData[group]->brightness(globalBrightness);
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
    if ((pixelData[group] != nullptr) &&
        pixelLock &&
        (xSemaphoreTakeRecursive(pixelLock, 0) == pdTRUE))
    {
        pixelData[group]->pixelRGB(pixelIndex, red, green, blue);
        xSemaphoreGiveRecursive(pixelLock);
    }
}

void pixels::setAll(pixelGroup_t group,
                    uint8_t red,
                    uint8_t green,
                    uint8_t blue)
{
    if ((pixelData[group] != nullptr) &&
        pixelLock &&
        (xSemaphoreTakeRecursive(pixelLock, 0) == pdTRUE))
    {
        pixelData[group]->pixelRangeRGB(0, 255, red, green, blue);
        xSemaphoreGiveRecursive(pixelLock);
    }
}

void pixels::shiftToNext(pixelGroup_t group)
{
    if ((pixelData[group] != nullptr) &&
        pixelLock &&
        (xSemaphoreTakeRecursive(pixelLock, 0) == pdTRUE))
    {
        pixelData[group]->shiftToNext();
        xSemaphoreGiveRecursive(pixelLock);
    }
}

void pixels::shiftToPrevious(pixelGroup_t group)
{
    if ((pixelData[group] != nullptr) &&
        pixelLock &&
        (xSemaphoreTakeRecursive(pixelLock, 0) == pdTRUE))
    {
        pixelData[group]->shiftToPrevious();
        xSemaphoreGiveRecursive(pixelLock);
    }
}

void pixels::reset()
{
    if (pixelLock &&
        (xSemaphoreTakeRecursive(pixelLock, 0) == pdTRUE))
    {
        for (int i = 0; i < 3; i++)
            if (pixelData[i] != nullptr)
            {
                pixelData[i]->pixelRangeRGB(0, 255, 0, 0, 0);
                pixelData[i]->show();
            }
        xSemaphoreGiveRecursive(pixelLock);
    }
}

void pixels::show()
{
    if (pixelLock &&
        (xSemaphoreTakeRecursive(pixelLock, 0) == pdTRUE))
    {
        for (int i = 0; i < 3; i++)
            if (pixelData[i] != nullptr)
                pixelData[i]->show();
        xSemaphoreGiveRecursive(pixelLock);
    }
}

// ----------------------------------------------------------------------------
// Notifications
// ----------------------------------------------------------------------------

void PixelControlNotification::onStart()
{
    if (pixelLock &&
        (xSemaphoreTakeRecursive(pixelLock, WAIT_TICKS) == pdTRUE))
    {
        pixelControl_OnStart();
        xSemaphoreGiveRecursive(pixelLock);
    }
}

void PixelControlNotification::onBitePoint()
{
    if (pixelLock &&
        (xSemaphoreTakeRecursive(pixelLock, WAIT_TICKS) == pdTRUE))
    {
        pixelControl_OnBitePoint();
        xSemaphoreGiveRecursive(pixelLock);
    }
}

void PixelControlNotification::onConnected()
{
    if (pixelLock &&
        (xSemaphoreTakeRecursive(pixelLock, WAIT_TICKS) == pdTRUE))
    {
        pixelControl_OnConnected();
        xSemaphoreGiveRecursive(pixelLock);
    }
}

void PixelControlNotification::onBLEdiscovering()
{
    if (pixelLock &&
        (xSemaphoreTakeRecursive(pixelLock, WAIT_TICKS) == pdTRUE))
    {
        pixelControl_OnBLEdiscovering();
        xSemaphoreGiveRecursive(pixelLock);
    }
}

void PixelControlNotification::onLowBattery()
{
    if (pixelLock &&
        (xSemaphoreTakeRecursive(pixelLock, WAIT_TICKS) == pdTRUE))
    {
        pixelControl_OnLowBattery();
        xSemaphoreGiveRecursive(pixelLock);
    }
}

// ----------------------------------------------------------------------------

void PixelControlNotification::pixelControl_OnStart()
{
    // All white
    pixels::setAll(GRP_TELEMETRY, 85, 85, 85);
    pixels::setAll(GRP_BUTTONS, 85, 85, 85);
    pixels::setAll(GRP_INDIVIDUAL, 85, 85, 85);
    pixels::show();
    vTaskDelay(pdMS_TO_TICKS(1000));
}

#define CEIL_DIV(dividend, divisor) (dividend + divisor - 1) / divisor

void PixelControlNotification::pixelControl_OnBitePoint()
{
    uint8_t pixelCount = pixels::getPixelCount(GRP_TELEMETRY);
    uint8_t litCount = CEIL_DIV(userSettings::bitePoint * pixelCount, CLUTCH_FULL_VALUE);
    pixels::setAll(GRP_TELEMETRY, 0, 0, 0);
    pixels::setAll(GRP_BUTTONS, 0, 0, 0);
    pixels::setAll(GRP_INDIVIDUAL, 0, 0, 0);
    for (int i = 0; i < litCount; i++)
        pixels::set(GRP_TELEMETRY, i, 85, 85, 0);
    pixels::show();
    vTaskDelay(pdMS_TO_TICKS(250));
    pixels::setAll(GRP_TELEMETRY, 0, 0, 0);
    pixels::show();
}

void PixelControlNotification::pixelControl_OnConnected()
{
    pixels::reset();
}

void PixelControlNotification::pixelControl_OnBLEdiscovering()
{
    // All purple
    pixels::setAll(GRP_TELEMETRY, 85, 0, 85);
    pixels::setAll(GRP_BUTTONS, 85, 0, 85);
    pixels::setAll(GRP_INDIVIDUAL, 85, 0, 85);
    pixels::show();
    vTaskDelay(pdMS_TO_TICKS(250));
}

void PixelControlNotification::pixelControl_OnLowBattery()
{
    // Alternate pixels in blue and red
    for (int group = 0; group < 3; group++)
    {
        uint8_t pixelCount = pixels::getPixelCount((pixelGroup_t)group);
        for (int pixelIndex = 0; pixelIndex < pixelCount; pixelIndex = pixelIndex + 2)
            pixels::set((pixelGroup_t)group, pixelIndex, 127, 0, 0); // red
        for (int pixelIndex = 1; pixelIndex < pixelCount; pixelIndex = pixelIndex + 2)
            pixels::set((pixelGroup_t)group, pixelIndex, 0, 0, 127); // blue
    }
    pixels::show();
    // Cool animation
    for (int animCount = 0; animCount < 5; animCount++)
    {
        vTaskDelay(pdMS_TO_TICKS(200));
        pixels::shiftToNext(GRP_TELEMETRY);
        pixels::shiftToNext(GRP_BUTTONS);
        pixels::shiftToNext(GRP_INDIVIDUAL);
        pixels::show();
    }
    vTaskDelay(pdMS_TO_TICKS(200));
    pixels::setAll(GRP_TELEMETRY, 0, 0, 0);
    pixels::setAll(GRP_BUTTONS, 0, 0, 0);
    pixels::setAll(GRP_INDIVIDUAL, 0, 0, 0);
    pixels::show();
}