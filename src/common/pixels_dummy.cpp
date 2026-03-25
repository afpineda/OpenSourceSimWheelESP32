/**
 * @file pixels_dummy.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-25
 * @brief Dummy implementation of pixel control
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"

static std::recursive_mutex pixelMutex;

void pixels::configure(
    PixelGroup group,
    OutputGPIO dataPin,
    uint8_t pixelCount,
    bool useLevelShift,
    PixelDriver driver,
    uint8_t globalBrightness,
    bool reverse) {}
void internals::pixels::getReady() {}
uint8_t internals::pixels::getCount(PixelGroup group) { return 16; }

void internals::pixels::show(
    const ::std::vector<Pixel> &telemetry,
    const ::std::vector<Pixel> &buttons,
    const ::std::vector<Pixel> &individual) {}