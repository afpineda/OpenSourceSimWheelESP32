/**
 * @file pixels_fake.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-25
 * @brief Fake implementation of pixel control
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include <mutex>

static std::recursive_mutex pixelMutex;

#define PIXEL_COUNT 16

::std::vector<Pixel> fake_telemetry(PIXEL_COUNT);
::std::vector<Pixel> fake_buttons(PIXEL_COUNT);
::std::vector<Pixel> fake_individual(PIXEL_COUNT);

::std::vector<Pixel> &fake_pixels(PixelGroup grp)
{
    if (grp == PixelGroup::GRP_INDIVIDUAL)
        return fake_individual;
    else if (grp == PixelGroup::GRP_BUTTONS)
        return fake_buttons;
    else
        return fake_telemetry;
}

void pixels::configure(
    PixelGroup group,
    OutputGPIO dataPin,
    uint8_t pixelCount,
    bool useLevelShift,
    PixelDriver driver,
    uint8_t globalBrightness,
    bool reverse)
{
}

void internals::pixels::getReady() {}

uint8_t internals::pixels::getCount(PixelGroup group)
{
    return PIXEL_COUNT;
}

//---------------------------------------------------------------

void internals::pixels::show(
    const ::std::vector<Pixel> &telemetry,
    const ::std::vector<Pixel> &buttons,
    const ::std::vector<Pixel> &individual)
{
    pixelMutex.lock();
    fake_telemetry = telemetry;
    fake_buttons = buttons;
    fake_individual = individual;
    pixelMutex.unlock();
}
