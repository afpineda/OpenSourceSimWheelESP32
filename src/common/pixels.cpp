/**
 * @file Pixels.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-12-13
 * @brief Everything related to pixel control.
 *
 * @copyright Licensed under the EUPL
 *
 */

//---------------------------------------------------------------
// Imports
//---------------------------------------------------------------

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include "OutputHardware.hpp"

#include <mutex>
#include <chrono>
#include <thread>

//---------------------------------------------------------------
// Globals
//---------------------------------------------------------------

static LEDStrip *pixelData[3] = {nullptr};
#define DELAY_MS(ms) std::this_thread::sleep_for(std::chrono::milliseconds(ms))
#define WAIT_MS std::chrono::milliseconds(80)
#define INT(N) ((uint8_t)N)
#define CEIL_DIV(dividend, divisor) (dividend + divisor - 1) / divisor

//---------------------------------------------------------------
// The mutex issue
//---------------------------------------------------------------
// DEVELOPMENT NOTE 2025/07/13:
// When using a recursive timed mutex
// an "unable to allocate buffer" error shows up repeatedly
// leading to a system crash.
// This happens both with std::recursive_timed_mutex (c++ stdlib)
// and xSemaphoreCreateRecursiveMutexStatic (FreeRTOS).
// Current implementation uses std::recursive_mutex instead,
// which seems bug-free.
// Some macros are defined for easy rework in case the
// mutex implementation has to change in the future.
//---------------------------------------------------------------

static std::recursive_mutex pixelMutex;
#define CAN_TAKE_MUTEX pixelMutex.try_lock()
#define TAKE_MUTEX pixelMutex.lock()
#define GIVE_MUTEX pixelMutex.unlock()

//---------------------------------------------------------------
//---------------------------------------------------------------
// Public
//---------------------------------------------------------------
//---------------------------------------------------------------

void pixels::configure(
    PixelGroup group,
    OutputGPIO dataPin,
    uint8_t pixelCount,
    bool useLevelShift,
    PixelDriver pixelType,
    PixelFormat pixelFormat,
    uint8_t globalBrightness)
{
    if (pixelData[INT(group)] != nullptr)
        throw std::runtime_error("A pixel group was configured twice");
    pixelData[INT(group)] = new LEDStrip(
        dataPin,
        pixelCount,
        useLevelShift,
        pixelType,
        pixelFormat);
    pixelData[INT(group)]->brightness(globalBrightness);
}

//---------------------------------------------------------------
//---------------------------------------------------------------
// Internal
//---------------------------------------------------------------
//---------------------------------------------------------------

void pixelsShutdown()
{
    // NOTE: no timeouts here.
    // Shutdown is mandatory.
    TAKE_MUTEX;
    for (int i = 0; i < 3; i++)
        if (pixelData[i])
        {
            pixelData[i]->clear();
            pixelData[i]->show();
            delete pixelData[i];
            pixelData[i] = nullptr;
        }
    GIVE_MUTEX;
}

//---------------------------------------------------------------

void internals::pixels::getReady()
{
    OnShutdown::subscribe(pixelsShutdown);
}

//---------------------------------------------------------------

uint8_t internals::pixels::getCount(PixelGroup group)
{
    if (pixelData[INT(group)] != nullptr)
        return pixelData[INT(group)]->getPixelCount();
    else
        return 0;
}

//---------------------------------------------------------------
// Pixel control
//---------------------------------------------------------------

void internals::pixels::set(
    PixelGroup group,
    uint8_t pixelIndex,
    uint8_t red,
    uint8_t green,
    uint8_t blue)
{
    if (pixelData[INT(group)] && CAN_TAKE_MUTEX)
    {
        pixelData[INT(group)]->pixelRGB(pixelIndex, red, green, blue);
        GIVE_MUTEX;
    }
}

void internals::pixels::setAll(
    PixelGroup group,
    uint8_t red,
    uint8_t green,
    uint8_t blue)
{
    if (pixelData[INT(group)] && CAN_TAKE_MUTEX)
    {
        pixelData[INT(group)]->pixelRangeRGB(0, 255, red, green, blue);
        GIVE_MUTEX;
    }
}

void internals::pixels::shiftToNext(PixelGroup group)
{
    if (pixelData[INT(group)] && CAN_TAKE_MUTEX)
    {
        pixelData[INT(group)]->shiftToNext();
        GIVE_MUTEX;
    }
}

void internals::pixels::shiftToPrevious(PixelGroup group)
{
    if (pixelData[INT(group)] && CAN_TAKE_MUTEX)
    {
        pixelData[INT(group)]->shiftToPrevious();
        GIVE_MUTEX;
    }
}

void internals::pixels::reset()
{
    if (CAN_TAKE_MUTEX)
    {
        for (int i = 0; i < 3; i++)
            if (pixelData[i])
            {
                pixelData[i]->pixelRangeRGB(0, 255, 0, 0, 0);
                pixelData[i]->show();
            }
        GIVE_MUTEX;
    }
}

void internals::pixels::show()
{
    if (CAN_TAKE_MUTEX)
    {
        for (int i = 0; i < 3; i++)
            if (pixelData[i])
            {
                pixelData[i]->show();
            }
        GIVE_MUTEX;
    }
}

//---------------------------------------------------------------
//---------------------------------------------------------------
// Notifications
//---------------------------------------------------------------
//---------------------------------------------------------------

//---------------------------------------------------------------
// Protected methods available to descendant classes
//---------------------------------------------------------------

uint8_t PixelControlNotification::getPixelCount(PixelGroup group)
{
    return internals::pixels::getCount(group);
}

void PixelControlNotification::set(
    PixelGroup group,
    uint8_t pixelIndex,
    uint8_t red,
    uint8_t green,
    uint8_t blue)
{
    internals::pixels::set(group, pixelIndex, red, green, blue);
}

void PixelControlNotification::setAll(
    PixelGroup group,
    uint8_t red,
    uint8_t green,
    uint8_t blue)
{
    internals::pixels::setAll(group, red, green, blue);
}

void PixelControlNotification::shiftToNext(PixelGroup group)
{
    internals::pixels::shiftToNext(group);
}

void PixelControlNotification::shiftToPrevious(PixelGroup group)
{
    internals::pixels::shiftToPrevious(group);
}

bool PixelControlNotification::renderBatteryLevel(
    PixelGroup group,
    bool colorGradientOrPercentage,
    uint32_t barColor)
{
    if (BatteryService::call::isBatteryPresent())
    {
        int soc = BatteryService::call::getLastBatteryLevel();
        if (colorGradientOrPercentage)
        {
            // Color gradient
            uint8_t green = (255 * soc) / 100;
            internals::pixels::setAll(group, 255 - green, green, 0);
        }
        else
        {
            // Percentage bar
            uint8_t pixelCount = internals::pixels::getCount(group);
            uint8_t litCount = (soc * pixelCount) / 100;
            if (litCount == 0)
                // At least, one pixel must be shown, otherwise
                // there is no SoC notification at all
                litCount = 1;
            uint8_t blue = barColor;
            uint8_t green = (barColor >> 8);
            uint8_t red = (barColor >> 16);
            for (uint8_t pixelIndex = 0; pixelIndex < litCount; pixelIndex++)
                internals::pixels::set(group, pixelIndex, red, green, blue);
        }
        return true;
    }
    return false;
}

//---------------------------------------------------------------
// Inherited virtual method implementation
//---------------------------------------------------------------

void PixelControlNotification::onStart()
{
    notConnectedYet = true;
    TAKE_MUTEX;
    pixelControl_OnStart();
    GIVE_MUTEX;
}

void PixelControlNotification::onBitePoint(uint8_t bitePoint)
{
    TAKE_MUTEX;
    pixelControl_OnBitePoint(bitePoint);
    GIVE_MUTEX;
}

void PixelControlNotification::onConnected()
{
    notConnectedYet = false;
    TAKE_MUTEX;
    pixelControl_OnConnected();
    GIVE_MUTEX;
}

void PixelControlNotification::onBLEdiscovering()
{
    notConnectedYet = true;
    TAKE_MUTEX;
    pixelControl_OnBLEdiscovering();
    GIVE_MUTEX;
}

void PixelControlNotification::onLowBattery()
{
    TAKE_MUTEX;
    pixelControl_OnLowBattery();
    GIVE_MUTEX;
}

void PixelControlNotification::onSaveSettings()
{
    TAKE_MUTEX;
    pixelControl_OnSaveSettings();
    GIVE_MUTEX;
}

//---------------------------------------------------------------
// Default implementation of pixel control notifications
//---------------------------------------------------------------

void PixelControlNotification::pixelControl_OnStart()
{
    if (renderBatteryLevel(PixelGroup::GRP_TELEMETRY, false))
    {
        // Show battery level
        renderBatteryLevel(PixelGroup::GRP_BUTTONS, true);
        renderBatteryLevel(PixelGroup::GRP_INDIVIDUAL, true);
    }
    else
    {
        // There is no battery
        // All white
        internals::pixels::setAll(PixelGroup::GRP_TELEMETRY, 85, 85, 85);
        internals::pixels::setAll(PixelGroup::GRP_BUTTONS, 85, 85, 85);
        internals::pixels::setAll(PixelGroup::GRP_INDIVIDUAL, 85, 85, 85);
    }
    internals::pixels::show();
    DELAY_MS(1500);
}

void PixelControlNotification::pixelControl_OnBitePoint(uint8_t bitePoint)
{
    if (notConnectedYet)
        // Ignore the bite point event if not connected yet.
        // On startup, a single bite point event is always triggered
        // from the storage subsystem.
        return;

    uint8_t pixelCount = internals::pixels::getCount(PixelGroup::GRP_TELEMETRY);
    uint8_t litCount = CEIL_DIV(bitePoint * pixelCount, CLUTCH_FULL_VALUE);
    internals::pixels::setAll(PixelGroup::GRP_TELEMETRY, 0, 0, 0);
    internals::pixels::setAll(PixelGroup::GRP_BUTTONS, 0, 0, 0);
    internals::pixels::setAll(PixelGroup::GRP_INDIVIDUAL, 0, 0, 0);
    for (int i = 0; i < litCount; i++)
        internals::pixels::set(PixelGroup::GRP_TELEMETRY, i, 85, 85, 0);
    internals::pixels::show();
    DELAY_MS(250);

    if (notConnectedYet)
        pixelControl_OnBLEdiscovering();
    else
    {
        internals::pixels::setAll(PixelGroup::GRP_TELEMETRY, 0, 0, 0);
        internals::pixels::show();
    }
}

void PixelControlNotification::pixelControl_OnConnected()
{
    internals::pixels::reset();
}

void PixelControlNotification::pixelControl_OnBLEdiscovering()
{
    // All purple
    internals::pixels::setAll(PixelGroup::GRP_TELEMETRY, 85, 0, 85);
    internals::pixels::setAll(PixelGroup::GRP_BUTTONS, 85, 0, 85);
    internals::pixels::setAll(PixelGroup::GRP_INDIVIDUAL, 85, 0, 85);
    internals::pixels::show();
    DELAY_MS(250);
}

void PixelControlNotification::pixelControl_OnLowBattery()
{
    // Alternate pixels in blue and red
    for (int group = 0; group < 3; group++)
    {
        uint8_t pixelCount = internals::pixels::getCount((PixelGroup)group);
        for (int pixelIndex = 0; pixelIndex < pixelCount; pixelIndex = pixelIndex + 2)
            internals::pixels::set((PixelGroup)group, pixelIndex, 127, 0, 0); // red
        for (int pixelIndex = 1; pixelIndex < pixelCount; pixelIndex = pixelIndex + 2)
            internals::pixels::set((PixelGroup)group, pixelIndex, 0, 0, 127); // blue
    }
    internals::pixels::show();
    // Cool animation
    for (int animCount = 0; animCount < 5; animCount++)
    {
        DELAY_MS(200);
        internals::pixels::shiftToNext(PixelGroup::GRP_TELEMETRY);
        internals::pixels::shiftToNext(PixelGroup::GRP_BUTTONS);
        internals::pixels::shiftToNext(PixelGroup::GRP_INDIVIDUAL);
        internals::pixels::show();
    }
    DELAY_MS(200);

    if (notConnectedYet)
        pixelControl_OnBLEdiscovering();
    else
        internals::pixels::reset();
}

void PixelControlNotification::pixelControl_OnSaveSettings()
{
    // All green
    internals::pixels::setAll(PixelGroup::GRP_TELEMETRY, 0, 85, 0);
    internals::pixels::setAll(PixelGroup::GRP_BUTTONS, 0, 85, 0);
    internals::pixels::setAll(PixelGroup::GRP_INDIVIDUAL, 0, 85, 0);
    internals::pixels::show();
    DELAY_MS(150);
    // All off
    internals::pixels::reset();
    DELAY_MS(150);
    // All green
    internals::pixels::setAll(PixelGroup::GRP_TELEMETRY, 0, 85, 0);
    internals::pixels::setAll(PixelGroup::GRP_BUTTONS, 0, 85, 0);
    internals::pixels::setAll(PixelGroup::GRP_INDIVIDUAL, 0, 85, 0);
    internals::pixels::show();
    DELAY_MS(150);
    // All off
    internals::pixels::reset();

    if (notConnectedYet)
        pixelControl_OnBLEdiscovering();
    else
        internals::pixels::reset();
}