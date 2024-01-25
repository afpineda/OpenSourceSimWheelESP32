/**
 * @file hidImplementation_USB.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-10-24
 * @brief Implementation of a HID device through the tinyUSB stack
 *
 * @copyright Licensed under the EUPL
 *
 */

// Implementation inspired by
// https://github.com/espressif/arduino-esp32/blob/master/libraries/USB/examples/CustomHIDDevice/CustomHIDDevice.ino
//
// Use this app for testing:
// http://www.planetpointy.co.uk/joystick-test-application/

#include "SimWheel.h"
#include "HID_definitions.h"
#include "USB.h"
#include "USBHID.h"

// ----------------------------------------------------------------------------
// USB classes
// ----------------------------------------------------------------------------

class SimWheelHIDImpl : public USBHIDDevice
{
    inputNumber_t selectedInput = UNSPECIFIED_INPUT_NUMBER;

    virtual uint16_t _onGetDescriptor(uint8_t *buffer) override
    {
        memcpy(buffer, hid_descriptor, sizeof(hid_descriptor));
        return sizeof(hid_descriptor);
    }

    virtual uint16_t _onGetFeature(uint8_t report_id, uint8_t *buffer, uint16_t len) override
    {
        return hidImplementation::common::onGetFeature(report_id,buffer,len);
    }

    virtual void _onSetFeature(uint8_t report_id, const uint8_t *buffer, uint16_t len) override
    {
        hidImplementation::common::onSetFeature(report_id,buffer,len);
    }
};

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

static bool notifyConfigChanges = false;
USBHID hid;
SimWheelHIDImpl simWheelHID;

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void hidImplementation::begin(
    std::string deviceName,
    std::string deviceManufacturer,
    bool enableAutoPowerOff)
{
    if (!hid.ready())
    {
        USB.productName(deviceName.c_str());
        USB.manufacturerName(deviceManufacturer.c_str());
        USB.usbClass(0x03); // HID device class
        USB.usbSubClass(0); // No subclass
        hid.addDevice(&simWheelHID, sizeof(hid_descriptor));
        hid.begin();
        USB.begin();
        notify::connected();
        hidImplementation::reset();
    }
}

// ----------------------------------------------------------------------------
// HID profile
// ----------------------------------------------------------------------------

void hidImplementation::reset()
{
    if (hid.ready())
    {
        uint8_t report[GAMEPAD_REPORT_SIZE];
        hidImplementation::common::onReset(report);
        hid.SendReport(RID_INPUT_GAMEPAD, report, GAMEPAD_REPORT_SIZE);
    }
}

void hidImplementation::reportInput(
    inputBitmap_t inputsLow,
    inputBitmap_t inputsHigh,
    uint8_t POVstate,
    clutchValue_t leftAxis,
    clutchValue_t rightAxis,
    clutchValue_t clutchAxis)
{
    if (hid.ready())
    {
        uint8_t report[GAMEPAD_REPORT_SIZE];
        hidImplementation::common::onReportInput(
            report,
            notifyConfigChanges,
            inputsLow,
            inputsHigh,
            POVstate,
            leftAxis,
            rightAxis,
            clutchAxis);
        hid.SendReport(RID_INPUT_GAMEPAD, report, GAMEPAD_REPORT_SIZE);
    }
}

void hidImplementation::reportBatteryLevel(int level)
{
    // Do nothing
}

void hidImplementation::reportChangeInConfig()
{
    notifyConfigChanges = true; // Will be reported in next input report
}

// ----------------------------------------------------------------------------
// Status
// ----------------------------------------------------------------------------

bool hidImplementation::isConnected()
{
    return hid.ready();
}
