/**
 * @file Report2Test.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-10
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
#include <cinttypes>
#include <cassert>

//-------------------------------------------------------------------
// Mocks
//-------------------------------------------------------------------

class UIServiceMock : public UIService
{
    virtual uint8_t getMaxFPS() override
    {
        return 33;
    }
};

//-------------------------------------------------------------------
// Auxiliary
//-------------------------------------------------------------------

struct __attribute__((packed))
{
    uint16_t magic_num;
    uint16_t major;
    uint16_t minor;
    uint16_t flags;
    uint64_t id = 99ULL;
    uint8_t maxFPS;
    uint8_t count1;
    uint8_t count2;
    uint8_t count3;
} report2;

uint8_t *report2bytes = (uint8_t *)&report2;

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------
//-------------------------------------------------------------------

int main()
{
    DeviceCapabilities::setFlag(DeviceCapability::ROTARY_ENCODERS);
    DeviceCapabilities::setFlag(DeviceCapability::DPAD);
    assert((sizeof(report2) == CAPABILITIES_REPORT_SIZE) && "Test is outdated");
    uint16_t expectedFlags = DeviceCapabilities::getFlags();
    UIService::inject(new UIServiceMock);

    internals::hid::common::onGetFeature(
        RID_FEATURE_CAPABILITIES,
        report2bytes,
        sizeof(report2));

    assert((report2.magic_num == 0xBF51) && "Bad magic number");
    assert((report2.major == DATA_MAJOR_VERSION) && "Bad major version");
    assert((report2.flags == expectedFlags) && "Bad flags");
    assert((report2.id == 0ULL) && "Bad ID");
    assert((report2.maxFPS == 33) && "Bad FPS");
    assert((report2.count1 == 16) && "Bad pixel count 1");
    assert((report2.count2 == 16) && "Bad pixel count 2");
    assert((report2.count3 == 16) && "Bad pixel count 3");

    report2.id = 999UL;
    internals::hid::common::onSetFeature(
        RID_FEATURE_CAPABILITIES,
        report2bytes,
        sizeof(report2));
    internals::hid::common::onGetFeature(
        RID_FEATURE_CAPABILITIES,
        report2bytes,
        sizeof(report2));
        assert((report2.id == 0ULL) && "Report 2 is not read-only");

    return 0;
}