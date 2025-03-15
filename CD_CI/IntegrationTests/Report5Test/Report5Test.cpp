/**
 * @file Report5Test.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-03-01
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
#include <iostream>

//-------------------------------------------------------------------
// Auxiliary
//-------------------------------------------------------------------

typedef struct __attribute__((packed))
{
    uint16_t vid = 0xFFFF;
    uint16_t pid = 0xFFFF;
    uint16_t cc = 0xFFFF;
} Report5;

#define REPORT5BYTES(s) ((uint8_t *)&s)
extern void commonHidStart();
#define FACTORY_VID 1000
#define FACTORY_PID 2000

void printReport5(Report5 &item)
{
    std::cout << "vid = " << item.vid;
    std::cout << " pid = " << item.pid;
    std::cout << std::endl;
}

//-------------------------------------------------------------------
// Mocks
//-------------------------------------------------------------------

static Report5 custom;

class HidMock : public HidService
{
public:
    virtual void getCustomHardwareID(
        uint16_t &customVID,
        uint16_t &customPID) override
    {
        customVID = custom.vid;
        customPID = custom.pid;
    }

    virtual void setCustomHardwareID(
        uint16_t customVID,
        uint16_t customPID,
        bool save) override
    {
        custom.vid = customVID;
        custom.pid = customPID;
    }
} hidMock;

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------
//-------------------------------------------------------------------

int main()
{
    assert((sizeof(Report5) == HARDWARE_ID_REPORT_SIZE) && "Test is outdated");
    HidService::inject(&hidMock);

    // This simulates no stored hardware ID
    custom.vid = 0;
    custom.pid = 0;

    // Configure factory hardware ID
    hid::configure("", "", false, FACTORY_VID, FACTORY_PID);

    // Since there is no stored custom ID, the factory ID should be used
    commonHidStart();

    Report5 r5;

    // At read, should retrieve the factory hardware ID
    internals::hid::common::onGetFeature(RID_FEATURE_HARDWARE_ID, REPORT5BYTES(r5), sizeof(Report5));
    assert(
        ((r5.vid == FACTORY_VID) && (r5.pid == FACTORY_PID)) &&
        "Invalid factory hw ID");

    // Set wrong control code
    r5.pid = 12;
    r5.vid = 12;
    r5.cc = 0;
    internals::hid::common::onSetFeature(RID_FEATURE_HARDWARE_ID, REPORT5BYTES(r5), sizeof(Report5));
    assert(
        ((custom.vid != r5.vid) && (custom.pid != r5.pid)) &&
        "Control code failed");

    // Set valid control code
    r5.vid = 0xEFEF;
    r5.pid = 0xFEFE;
    r5.cc = (r5.pid * r5.vid) % 65536;
    internals::hid::common::onSetFeature(RID_FEATURE_HARDWARE_ID, REPORT5BYTES(r5), sizeof(Report5));
    assert(
        ((custom.vid == r5.vid) && (custom.pid == r5.pid)) &&
        "Failed to set a custom hw ID");

    // Read current hardware ID
    r5.vid = 0;
    r5.pid = 0;
    r5.cc = 0;
    internals::hid::common::onGetFeature(RID_FEATURE_HARDWARE_ID, REPORT5BYTES(r5), sizeof(Report5));
    assert(
        ((custom.vid == r5.vid) && (custom.pid == r5.pid)) &&
        "Failed to retrieve the custom hw ID");

    // Reset to factory defaults
    r5.pid = 0;
    r5.vid = 0;
    r5.cc = 0xAA96;
    internals::hid::common::onSetFeature(RID_FEATURE_HARDWARE_ID, REPORT5BYTES(r5), sizeof(Report5));
    assert(
        ((custom.vid == 0) && (custom.pid == 0)) &&
        "Invalid custom hw ID after reset to factory defaults");
}
