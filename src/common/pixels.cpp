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

static LEDStrip *led_strip[3] = {nullptr};
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
typedef ::std::lock_guard<std::recursive_mutex> PixelGuard;

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
    if (led_strip[INT(group)] != nullptr)
        throw std::runtime_error("A pixel group was configured twice");
    led_strip[INT(group)] = new LEDStrip(
        dataPin,
        pixelCount,
        useLevelShift,
        driver,
        reverse);
    led_strip[INT(group)]->brightness(globalBrightness);
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
        led_strip[i]->clear();
        delete led_strip[i];
        led_strip[i] = nullptr;
    }
    pixelMutex.unlock();
}

//---------------------------------------------------------------

void internals::pixels::getReady()
{
    OnShutdown.subscribe(pixelsShutdown);
}

//---------------------------------------------------------------

uint8_t internals::pixels::getCount(PixelGroup group)
{
    if (led_strip[INT(group)])
        return led_strip[INT(group)]->getPixelCount();
    else
        return 0;
}

//---------------------------------------------------------------

void internals::pixels::show(
    const ::std::vector<Pixel> &telemetry,
    const ::std::vector<Pixel> &buttons,
    const ::std::vector<Pixel> &individual)
{
    if (pixelMutex.try_lock())
    {
        if (led_strip[INT(PixelGroup::GRP_TELEMETRY)])
            led_strip[INT(PixelGroup::GRP_TELEMETRY)]->show(telemetry);
        if (led_strip[INT(PixelGroup::GRP_BUTTONS)])
            led_strip[INT(PixelGroup::GRP_BUTTONS)]->show(buttons);
        if (led_strip[INT(PixelGroup::GRP_INDIVIDUAL)])
            led_strip[INT(PixelGroup::GRP_INDIVIDUAL)]->show(individual);
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

uint8_t PixelControlNotification::getPixelCount(PixelGroup group) noexcept
{
    return internals::pixels::getCount(group);
}

void PixelControlNotification::show(
    PixelGroup group,
    const ::std::vector<Pixel> &data) noexcept
{
    if (led_strip[INT(group)])
        led_strip[INT(group)]->show(data);
}

static ::std::vector<Pixel> PixelControlNotification::pixelVector(
    PixelGroup group,
    Pixel color) noexcept
{
    if (led_strip[INT(group)])
        return led_strip[INT(group)]->pixelVector(color);
    else
        return PixelVector{};
}

static void PixelControlNotification::reset() noexcept
{
    for (int i = 0; i < 3; i++)
        if (led_strip[i])
            led_strip[i]->clear();
}

// bool PixelControlNotification::renderBatteryLevel(
//     ::std::vector<Pixel> &data,
//     bool colorGradientOrPercentage,
//     uint32_t barColor)
// {
//     if (BatteryService::call().isBatteryPresent())
//     {
//         int soc = BatteryService::call().getLastBatteryLevel();
//         if (colorGradientOrPercentage)
//         {
//             // Color gradient
//             Pixel p;
//             p.green = (255 * soc) / 100;
//             fill(data, p);
//         }
//         else
//         {
//             // Percentage bar
//             uint8_t pixelCount = data.size();
//             uint8_t litCount = (soc * pixelCount) / 100;
//             if (litCount == 0)
//                 // At least, one pixel must be shown, otherwise
//                 // there is no SoC notification at all
//                 litCount = 1;
//             for (uint8_t pixelIndex = 0; pixelIndex < litCount; pixelIndex++)
//                 data[pixelIndex] = barColor;
//             for (uint8_t pixelIndex = litCount;
//                  pixelIndex < pixelCount;
//                  pixelIndex++)
//                 data[pixelIndex] = 0;
//         }
//         return true;
//     }
//     return false;
// }

//---------------------------------------------------------------
// Inherited virtual method implementation
//---------------------------------------------------------------

void PixelControlNotification::onStart()
{
    notConnectedYet = true;
    PixelGuard guard(pixelMutex);
    pixelControl_OnStart();
}

void PixelControlNotification::onBitePoint(uint8_t bitePoint)
{
    PixelGuard guard(pixelMutex);
    pixelControl_OnBitePoint(bitePoint);
}

void PixelControlNotification::onConnected()
{
    notConnectedYet = false;
    PixelGuard guard(pixelMutex);
    pixelControl_OnConnected();
}

void PixelControlNotification::onBLEdiscovering()
{
    notConnectedYet = true;
    PixelGuard guard(pixelMutex);
    pixelControl_OnBLEdiscovering();
}

void PixelControlNotification::onLowBattery()
{
    PixelGuard guard(pixelMutex);
    pixelControl_OnLowBattery();
}

void PixelControlNotification::onSaveSettings()
{
    PixelGuard guard(pixelMutex);
    pixelControl_OnSaveSettings();
}

//---------------------------------------------------------------
// Default implementation of pixel control notifications
//---------------------------------------------------------------

void PixelControlNotification::pixelControl_OnStart()
{
    // ::std::vector<Pixel> telemetry(getPixelCount(PixelGroup::GRP_TELEMETRY));
    // if (renderBatteryLevel(telemetry, false))
    // {
    //     // Show battery level
    //     show(PixelGroup::GRP_TELEMETRY, telemetry);
    // }
    // else
    // {
    //     // There is no battery
    //     // All white
    //     internals::pixels::setAll(PixelGroup::GRP_TELEMETRY, 85, 85, 85);
    //     internals::pixels::setAll(PixelGroup::GRP_BUTTONS, 85, 85, 85);
    //     internals::pixels::setAll(PixelGroup::GRP_INDIVIDUAL, 85, 85, 85);
    // }
    // internals::pixels::show();
    // DELAY_MS(1500);
}

void PixelControlNotification::pixelControl_OnBitePoint(uint8_t bitePoint)
{
    if (notConnectedYet)
        // Ignore the bite point event if not connected yet.
        // On startup, a single bite point event is always triggered
        // from the storage subsystem.
        return;

    PixelVector telemetry =
        static_cast<PixelVector>(pixelVector(PixelGroup::GRP_TELEMETRY));
    PixelVector buttons =
        static_cast<PixelVector>(pixelVector(PixelGroup::GRP_BUTTONS));
    PixelVector individual =
        static_cast<PixelVector>(pixelVector(PixelGroup::GRP_INDIVIDUAL));
    uint8_t pixelCount = telemetry.size();
    uint8_t litCount = CEIL_DIV(bitePoint * pixelCount, CLUTCH_FULL_VALUE);
    for (int i = 0; i < litCount; i++)
        telemetry[i] = 0x555500;
    show(PixelGroup::GRP_TELEMETRY, telemetry);
    show(PixelGroup::GRP_BUTTONS, buttons);
    show(PixelGroup::GRP_INDIVIDUAL, individual);
    DELAY_MS(250);

    if (notConnectedYet)
        pixelControl_OnBLEdiscovering();
    else
    {
        telemetry.fill(0);
        show(PixelGroup::GRP_TELEMETRY, telemetry);
    }
}

void PixelControlNotification::pixelControl_OnConnected()
{
    internals::pixels::reset();
}

void PixelControlNotification::pixelControl_OnBLEdiscovering()
{
    // All purple
    PixelVector telemetry =
        static_cast<PixelVector>(pixelVector(PixelGroup::GRP_TELEMETRY,
                                             0x550055));
    PixelVector buttons =
        static_cast<PixelVector>(pixelVector(PixelGroup::GRP_BUTTONS,
                                             0x550055));
    PixelVector individual =
        static_cast<PixelVector>(pixelVector(PixelGroup::GRP_INDIVIDUAL,
                                             0x550055));
    show(PixelGroup::GRP_TELEMETRY, telemetry);
    show(PixelGroup::GRP_BUTTONS, buttons);
    show(PixelGroup::GRP_INDIVIDUAL, individual);
    DELAY_MS(250);
}

void PixelControlNotification::pixelControl_OnLowBattery()
{
    // Alternate pixels in blue and red
    PixelVector telemetry =
        static_cast<PixelVector>(pixelVector(PixelGroup::GRP_TELEMETRY,
                                             0x00001F));
    PixelVector buttons =
        static_cast<PixelVector>(pixelVector(PixelGroup::GRP_BUTTONS,
                                             0x00001F));
    PixelVector individual =
        static_cast<PixelVector>(pixelVector(PixelGroup::GRP_INDIVIDUAL,
                                             0x00001F));
    for (int i = 0; i < telemetry.size(); i += 2)
        telemetry[i] = 0x1F0000; // red;
    for (int i = 0; i < buttons.size(); i += 2)
        buttons[i] = 0x1F0000; // red;
    for (int i = 0; i < individual.size(); i += 2)
        individual[i] = 0x1F0000; // red;

    // Show
    show(PixelGroup::GRP_TELEMETRY, telemetry);
    show(PixelGroup::GRP_BUTTONS, buttons);
    show(PixelGroup::GRP_INDIVIDUAL, individual);

    // Cool animation
    for (int animCount = 0; animCount < 5; animCount++)
    {
        DELAY_MS(200);

        telemetry >> 1;
        buttons >> 1;
        individual >> 1;
        show(PixelGroup::GRP_TELEMETRY, telemetry);
        show(PixelGroup::GRP_BUTTONS, buttons);
        show(PixelGroup::GRP_INDIVIDUAL, individual);
    }
    DELAY_MS(200);

    if (notConnectedYet)
        pixelControl_OnBLEdiscovering();
    else
        reset();
}

void PixelControlNotification::pixelControl_OnSaveSettings()
{
    // All green
    PixelVector telemetry =
        static_cast<PixelVector>(pixelVector(PixelGroup::GRP_TELEMETRY,
                                             0x001F00));
    PixelVector buttons =
        static_cast<PixelVector>(pixelVector(PixelGroup::GRP_BUTTONS,
                                             0x001F00));
    PixelVector individual =
        static_cast<PixelVector>(pixelVector(PixelGroup::GRP_INDIVIDUAL,
                                             0x001F00));
    show(PixelGroup::GRP_TELEMETRY, telemetry);
    show(PixelGroup::GRP_BUTTONS, buttons);
    show(PixelGroup::GRP_INDIVIDUAL, individual);
    DELAY_MS(150);

    // All off
    reset();
    DELAY_MS(150);

    // All green
    show(PixelGroup::GRP_TELEMETRY, telemetry);
    show(PixelGroup::GRP_BUTTONS, buttons);
    show(PixelGroup::GRP_INDIVIDUAL, individual);
    DELAY_MS(150);

    // All off
    if (notConnectedYet)
        pixelControl_OnBLEdiscovering();
    else
        reset();
}