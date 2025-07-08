/**
 * @file hid_USB.cpp
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

#include "USB.h"
#include "USBHID.h"

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include "HID_definitions.hpp"
#include "esp_mac.h" // For esp_efuse_mac_get_default()
// #include <Arduino.h> // For debugging

// ----------------------------------------------------------------------------
// USB classes
// ----------------------------------------------------------------------------

class SimWheelHIDImpl : public USBHIDDevice
{
    virtual uint16_t _onGetDescriptor(uint8_t *buffer) override
    {
        memcpy(buffer, hid_descriptor, sizeof(hid_descriptor));
        return sizeof(hid_descriptor);
    }

    virtual uint16_t _onGetFeature(uint8_t report_id, uint8_t *buffer, uint16_t len) override
    {
        return internals::hid::common::onGetFeature(report_id, buffer, len);
    }

    virtual void _onSetFeature(uint8_t report_id, const uint8_t *buffer, uint16_t len) override
    {
        // Note: for unknown reasons, output reports trigger this callback instead of _onOutput()
        if (report_id >= RID_OUTPUT_POWERTRAIN)
            // Output report
            internals::hid::common::onOutput(report_id, buffer, len);
        else
            // Feature report
            internals::hid::common::onSetFeature(report_id, buffer, len);
    }

    virtual void _onOutput(uint8_t report_id, const uint8_t *buffer, uint16_t len) override
    {
        // Note: never gets called unless report_id is zero. Reason unknown.
        internals::hid::common::onOutput(report_id, buffer, len);
    }
};

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

static bool notifyConfigChanges = false;
USBHID hidDevice;
SimWheelHIDImpl simWheelHID;

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void internals::hid::begin(
    std::string deviceName,
    std::string deviceManufacturer,
    bool enableAutoPowerOff,
    uint16_t vendorID,
    uint16_t productID)
{
    if (!hidDevice.ready())
    {
        // Initialization
        USB.productName(deviceName.c_str());
        USB.manufacturerName(deviceManufacturer.c_str());
        USB.usbClass(0x03); // HID device class
        USB.usbSubClass(0); // No subclass
        uint64_t serialNumber;
        if (esp_efuse_mac_get_default((uint8_t *)(&serialNumber)) == ESP_OK)
        {
            char serialAsStr[9];
            snprintf(serialAsStr,9,"%08llX",serialNumber);
            USB.serialNumber(serialAsStr);
        }
        hidDevice.addDevice(&simWheelHID, sizeof(hid_descriptor));
        hidDevice.begin();
        USB.begin();
        OnConnected::notify();
        internals::hid::reset();
    }
}

// ----------------------------------------------------------------------------
// HID profile
// ----------------------------------------------------------------------------

void internals::hid::reset()
{
    if (hidDevice.ready())
    {
        uint8_t report[GAMEPAD_REPORT_SIZE];
        internals::hid::common::onReset(report);
        hidDevice.SendReport(RID_INPUT_GAMEPAD, report, GAMEPAD_REPORT_SIZE);
    }
}

void internals::hid::reportInput(
    uint64_t inputsLow,
    uint64_t inputsHigh,
    uint8_t POVstate,
    uint8_t leftAxis,
    uint8_t rightAxis,
    uint8_t clutchAxis)
{
    if (hidDevice.ready())
    {
        uint8_t report[GAMEPAD_REPORT_SIZE];
        internals::hid::common::onReportInput(
            report,
            notifyConfigChanges,
            inputsLow,
            inputsHigh,
            POVstate,
            leftAxis,
            rightAxis,
            clutchAxis);
        hidDevice.SendReport(RID_INPUT_GAMEPAD, report, GAMEPAD_REPORT_SIZE);
    }
}

void internals::hid::reportBatteryLevel(int level)
{
    // Do nothing
}

void internals::hid::reportChangeInConfig()
{
    notifyConfigChanges = true; // Will be reported in next input report
}

bool internals::hid::supportsCustomHardwareID() { return false; }

// ----------------------------------------------------------------------------
// Status
// ----------------------------------------------------------------------------

bool internals::hid::isConnected()
{
    return hidDevice.ready();
}
