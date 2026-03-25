/**
 * @file ReportPixelControlTest.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-03-03
 * @brief Integration test
 *
 * @copyright Licensed under the EUPL
 *
 */

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include "HID_definitions.hpp"
#include "cd_ci_assertions.hpp"

#include <cinttypes>
#include <cassert>
#include <iostream>

//-------------------------------------------------------------------
// Auxiliary
//-------------------------------------------------------------------

extern ::std::vector<Pixel> &fake_pixels(PixelGroup grp);

typedef struct __attribute__((packed))
{
    uint8_t group = 0;
    uint8_t index = 0;
    uint8_t blue = 0;
    uint8_t green = 0;
    uint8_t red = 0;
    uint8_t reserved = 0;
} Report30;

#define REPORT30BYTES(s) ((uint8_t *)&s)

void assertPixel(
    std::string msg,
    PixelGroup grp,
    uint8_t index,
    uint32_t color)
{
    ::std::vector<Pixel> &v = fake_pixels(grp);
    uint32_t found = v[index];
    assert<uint32_t>::equals(msg, color, found);
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------
//-------------------------------------------------------------------

using namespace telemetry;

int main()
{
    assert((sizeof(Report30) == PIXEL_REPORT_SIZE) && "Test is outdated");
    internals::hid::common::getReady();
    OnStart();

    Report30 r30;
    r30.group = 0;
    r30.index = 1;
    r30.red = 0x01;
    r30.green = 0x02;
    r30.blue = 0x03;
    internals::hid::common::onOutput(
        RID_OUTPUT_PIXEL,
        REPORT30BYTES(r30),
        sizeof(r30));

    r30.group = 1;
    r30.index = 2;
    r30.red = 0x02;
    r30.green = 0x03;
    r30.blue = 0x04;
    internals::hid::common::onOutput(
        RID_OUTPUT_PIXEL,
        REPORT30BYTES(r30),
        sizeof(r30));

    r30.group = 2;
    r30.index = 3;
    r30.red = 0x03;
    r30.green = 0x04;
    r30.blue = 0x05;
    internals::hid::common::onOutput(
        RID_OUTPUT_PIXEL,
        REPORT30BYTES(r30),
        sizeof(r30));

    r30.group = 0xFF; // show command
    internals::hid::common::onOutput(
        RID_OUTPUT_PIXEL,
        REPORT30BYTES(r30),
        sizeof(r30));
    assertPixel("Pixel 1", PixelGroup::GRP_TELEMETRY, 1, 0x010203);
    assertPixel("Pixel 2", PixelGroup::GRP_BUTTONS, 2, 0x020304);
    assertPixel("Pixel 3", PixelGroup::GRP_INDIVIDUAL, 3, 0x030405);

    r30.group = 0xFE;
    internals::hid::common::onOutput(
        RID_OUTPUT_PIXEL,
        REPORT30BYTES(r30),
        sizeof(r30));
    assertPixel("Pixel reset 1", PixelGroup::GRP_TELEMETRY, 1, 0);
    assertPixel("Pixel reset 2", PixelGroup::GRP_BUTTONS, 2, 0);
    assertPixel("Pixel reset 3", PixelGroup::GRP_INDIVIDUAL, 3, 0);

    return 0;
}
