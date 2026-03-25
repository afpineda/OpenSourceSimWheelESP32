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
//---------------------------------------------------------------

static std::recursive_mutex pixelMutex;

//---------------------------------------------------------------
// The task watchdog timer issue
//---------------------------------------------------------------
// DEVELOPMENT NOTE 2025/09/26:
// When using pixel control notifications, the
// system triggers the task watchdog timer at
// PixelControlNotification::onConnected()
// when the device is first paired (and only in such a situation).
// This causes a system reset when the device is about to be
// paired and the computer gets crazy.
// The reason is unknown but probably is due to another task
// being starved, so an explicit call to
// esp_task_wdt_reset() does not help.
// A small task delay (and context swap) do the trick.
// The following macro is defined for easy rework and
// better semantics.
//---------------------------------------------------------------

#define PREVENT_STARVATION vTaskDelay(10)

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
    PixelDriver driver,
    uint8_t globalBrightness,
    bool reverse)
{
    if (pixelData[INT(group)] != nullptr)
        throw std::runtime_error("A pixel group was configured twice");
    pixelData[INT(group)] = new LEDStrip(
        dataPin,
        pixelCount,
        useLevelShift,
        driver,
        reverse);
    pixelData[INT(group)]->brightness(globalBrightness);
}

//---------------------------------------------------------------
//---------------------------------------------------------------
// Internal
//---------------------------------------------------------------
//---------------------------------------------------------------

void pixelsShutdown()
{
    // Shutdown is mandatory.
    pixelMutex.lock();
    for (int i = 0; i < 3; i++)
    {
        pixelData[i]->clear();
        delete pixelData[i];
        pixelData[i] = nullptr;
    }
    pixelMutes.unlock();
}

//---------------------------------------------------------------

void internals::pixels::getReady()
{
    OnShutdown.subscribe(pixelsShutdown);
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

PixelGuard internals::pixels::acquire()
{
    return PixelGuard(pixelMutex);
}

//---------------------------------------------------------------

void internals::pixels::show(
    const ::std::vector<Pixel> &telemetry,
    const ::std::vector<Pixel> &buttons,
    const ::std::vector<Pixel> &individual)
{
    if (pixelMutex.try_lock())
    {
        if (pixelData[PixelGroup::GRP_TELEMETRY])
            pixelData[PixelGroup::GRP_TELEMETRY]->show(telemetry);
        if (pixelData[PixelGroup::GRP_BUTTONS])
            pixelData[PixelGroup::GRP_BUTTONS]->show(buttons);
        if (pixelData[PixelGroup::GRP_INDIVIDUAL])
            pixelData[PixelGroup::GRP_INDIVIDUAL]->show(individual);
        pixelMutex.unlock();
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

bool PixelControlNotification::renderBatteryLevel(
    PixelGroup group,
    bool colorGradientOrPercentage,
    uint32_t barColor)
{
    if (BatteryService::call().isBatteryPresent())
    {
        int soc = BatteryService::call().getLastBatteryLevel();
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
    auto guard = internals::pixels::acquire();
    pixelControl_OnStart();
}

void PixelControlNotification::onBitePoint(uint8_t bitePoint)
{
    auto guard = internals::pixels::acquire();
    pixelControl_OnBitePoint(bitePoint);
}

void PixelControlNotification::onConnected()
{
    notConnectedYet = false;
    auto guard = internals::pixels::acquire();
    pixelControl_OnConnected();
}

void PixelControlNotification::onBLEdiscovering()
{
    notConnectedYet = true;
    auto guard = internals::pixels::acquire();
    pixelControl_OnBLEdiscovering();
}

void PixelControlNotification::onLowBattery()
{
    auto guard = internals::pixels::acquire();
    pixelControl_OnLowBattery();
}

void PixelControlNotification::onSaveSettings()
{
    auto guard = internals::pixels::acquire();
    pixelControl_OnSaveSettings();
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