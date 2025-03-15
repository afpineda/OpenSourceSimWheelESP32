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

uint32_t lastPixelColor = 0;
PixelGroup lastPixelGroup = PixelGroup::GRP_TELEMETRY;
bool shown = false;
uint8_t lastPixelIndex = 0;

void internals::pixels::getReady() {}
void internals::pixels::set(PixelGroup group,
                            uint8_t pixelIndex,
                            uint8_t red,
                            uint8_t green,
                            uint8_t blue)
{
    lastPixelGroup = group;
    lastPixelColor = blue | (green << 8) | (red << 16);
    lastPixelIndex = pixelIndex;
}

void internals::pixels::setAll(PixelGroup group,
                               uint8_t red,
                               uint8_t green,
                               uint8_t blue)
{
    internals::pixels::set(group, 0, red, green, blue);
}

void internals::pixels::shiftToNext(PixelGroup group) {}
void internals::pixels::shiftToPrevious(PixelGroup group) {}
void internals::pixels::show()
{
    shown = true;
}

void internals::pixels::reset()
{
    lastPixelColor = 0;
    lastPixelGroup = PixelGroup::GRP_TELEMETRY;
    shown = false;
    lastPixelIndex = 0;
}

uint8_t internals::pixels::getCount(PixelGroup group) { return 16; }
