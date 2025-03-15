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

void internals::pixels::getReady() {}
void internals::pixels::set(PixelGroup group,
                            uint8_t pixelIndex,
                            uint8_t red,
                            uint8_t green,
                            uint8_t blue) {}
void internals::pixels::setAll(PixelGroup group,
                               uint8_t red,
                               uint8_t green,
                               uint8_t blue) {}
void internals::pixels::shiftToNext(PixelGroup group) {}
void internals::pixels::shiftToPrevious(PixelGroup group) {}
void internals::pixels::show() {}
void internals::pixels::reset() {}
uint8_t internals::pixels::getCount(PixelGroup group) { return 16; }
