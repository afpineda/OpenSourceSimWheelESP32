/**
 * @file hidImplementation_USB.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-10-24
 * @brief Implementation of a HID device through the tinyUSB stack
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
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

    virtual uint16_t _onGetDescriptor(uint8_t *buffer) override
    {
        memcpy(buffer, hid_descriptor, sizeof(hid_descriptor));
        return sizeof(hid_descriptor);
    };

    virtual uint16_t _onGetFeature(uint8_t report_id, uint8_t *buffer, uint16_t len) override
    {
        if ((report_id == RID_FEATURE_CAPABILITIES) && (len >= CAPABILITIES_REPORT_SIZE))
        {

            buffer[0] = MAGIC_NUMBER_LOW;
            buffer[1] = MAGIC_NUMBER_HIGH;
            *(uint16_t *)(buffer + 2) = DATA_MAJOR_VERSION;
            *(uint16_t *)(buffer + 4) = DATA_MINOR_VERSION;
            *(uint16_t *)(buffer + 6) = capabilities::flags;
            return CAPABILITIES_REPORT_SIZE;
        }
        else if ((report_id == RID_FEATURE_CONFIG) && (len >= CONFIG_REPORT_SIZE))
        {

            buffer[0] = (uint8_t)userSettings::cpWorkingMode;
            buffer[1] = (uint8_t)userSettings::altButtonsWorkingMode;
            buffer[2] = (uint8_t)userSettings::bitePoint;
            buffer[3] = (uint8_t)power::getLastBatteryLevel();
            return CONFIG_REPORT_SIZE;
        }
        else
            return 0;
    };

    virtual void _onSetFeature(uint8_t report_id, const uint8_t *buffer, uint16_t len) override
    {
        if ((report_id == RID_FEATURE_CONFIG) && (len >= CONFIG_REPORT_SIZE))
        {
            if ((len > 0) && (buffer[0] >= CF_CLUTCH) && (buffer[0] <= CF_BUTTON))
            {
                // clutch function
                userSettings::setCPWorkingMode((clutchFunction_t)buffer[0]);
            }
            if ((len > 1) && (buffer[1] != 0xff))
            {
                // ALT Buttons mode
                userSettings::setALTButtonsWorkingMode((bool)buffer[1]);
            }
            if ((len > 2) && ((clutchValue_t)buffer[2] >= CLUTCH_NONE_VALUE) && ((clutchValue_t)buffer[2] <= CLUTCH_FULL_VALUE))
            {
                // Bite point
                userSettings::setBitePoint((clutchValue_t)buffer[2]);
            }
            if ((len > 3) && (buffer[3] == (uint8_t)simpleCommands_t::CMD_AXIS_RECALIBRATE))
            {
                // Force analog axis recalibration
                inputs::recalibrateAxes();
            }
            if ((len > 3) && (buffer[3] == (uint8_t)simpleCommands_t::CMD_BATT_RECALIBRATE))
            {
                // Restart auto calibration algorithm
                batteryCalibration::restartAutoCalibration();
            }
        }
    };
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
        report[0] = 0;
        report[1] = 0;
        report[2] = 0;
        report[3] = 0;
        report[4] = 0;
        report[5] = 0;
        report[6] = 0;
        report[7] = 0;
        report[8] = 0;
        report[9] = 0;
        report[10] = 0;
        report[11] = 0;
        report[12] = 0;
        report[13] = 0;
        report[14] = 0;
        report[15] = 0;
        report[16] = CLUTCH_NONE_VALUE;
        report[17] = CLUTCH_NONE_VALUE;
        report[18] = CLUTCH_NONE_VALUE;
        report[19] = 0;
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
        report[0] = ((uint8_t *)&inputsLow)[0];
        report[1] = ((uint8_t *)&inputsLow)[1];
        report[2] = ((uint8_t *)&inputsLow)[2];
        report[3] = ((uint8_t *)&inputsLow)[3];
        report[4] = ((uint8_t *)&inputsLow)[4];
        report[5] = ((uint8_t *)&inputsLow)[5];
        report[6] = ((uint8_t *)&inputsLow)[6];
        report[7] = ((uint8_t *)&inputsLow)[7];
        report[8] = ((uint8_t *)&inputsHigh)[0];
        report[9] = ((uint8_t *)&inputsHigh)[1];
        report[10] = ((uint8_t *)&inputsHigh)[2];
        report[11] = ((uint8_t *)&inputsHigh)[3];
        report[12] = ((uint8_t *)&inputsHigh)[4];
        report[13] = ((uint8_t *)&inputsHigh)[5];
        report[14] = ((uint8_t *)&inputsHigh)[6];
        report[15] = ((uint8_t *)&inputsHigh)[7];
        report[16] = (uint8_t)clutchAxis;
        report[17] = (uint8_t)leftAxis;
        report[18] = (uint8_t)rightAxis;
        report[19] = POVstate;
        if (notifyConfigChanges)
        {
            report[19] |= (RID_FEATURE_CONFIG << 4);
            notifyConfigChanges = false;
        }
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
