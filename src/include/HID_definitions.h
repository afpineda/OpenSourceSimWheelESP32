/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-12-17
 * @brief Key definitions of the sim wheel as a HID device.
 *        Independent from transport layer.
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#ifndef __HID_DEFINITIONS_H__
#define __HID_DEFINITIONS_H__

#include "SimWheelTypes.h"

// Report ID's
#define RID_INPUT_GAMEPAD 0x01
#define RID_FEATURE_CAPABILITIES 0x02
#define RID_FEATURE_CONFIG 0x03

// Report sizes (bytes)
#define GAMEPAD_REPORT_SIZE 20
#define CAPABILITIES_REPORT_SIZE 8
#define CONFIG_REPORT_SIZE 4

// GAME CONTROLLER APPEARANCES
#define CONTROLLER_TYPE_GAMEPAD 0x05
#define CONTROLLER_TYPE_JOYSTICK 0x04

// INPUT REPORT constants
#define BUTTON_COUNT 128

// PNP identifiers

//   USB NOT IMPLEMENTED
// See http://wiki.openmoko.org/wiki/USB_Product_IDs
// #define USB_VENDOR_ID // Not defined on purpose
// #define USB_PRODUCT_ID 0xffff

//   BLE
#define BLE_VENDOR_SOURCE 0x00
#define BLE_VENDOR_ID 0x1d50
#define BLE_PRODUCT_ID 0xffff

// Hardware revision
#define PRODUCT_REVISION 0x01

// Data specification version
#define DATA_MAJOR_VERSION 1
#define DATA_MINOR_VERSION 0

// Magic number, do not change
#define MAGIC_NUMBER_LOW 0x51
#define MAGIC_NUMBER_HIGH 0xBF

// HID report descriptor

static const uint8_t hid_descriptor[] = {
    0x05, 0x01,                    // UsagePage(Generic Desktop[1])
    0x09, CONTROLLER_TYPE_GAMEPAD, // UsageId
    0xA1, 0x01,                    // Collection(Application)

    // ___ INPUT REPORT ___
    0x85, RID_INPUT_GAMEPAD, //     ReportId

    //     Buttons (128 bits=16 bytes)
    0x05, 0x09, //     UsagePage(Button[9])
    0x19, 0x01, //     UsageIdMin(Button 1[1])
    0x29, 0x80, //     UsageIdMax(Button 128[128])
    0x15, 0x00, //     LogicalMinimum(0)
    0x25, 0x01, //     LogicalMaximum(1)
    0x95, 0x80, //     ReportCount(128)
    0x75, 0x01, //     ReportSize(1)
    0x81, 0x02, //     Input(Data, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)

    //     axis (1 byte)
    0x05, 0x01, //     UsagePage(Generic Desktop[1])
    0x09, 0x35, //     UsageId(Rz[53])
    // 0x15, 0x00, //     LogicalMinimum
    // 0x25, CLUTCH_FULL_VALUE, //     LogicalMaximum
    0x26, 0xFE, 0x00, //     LogicalMaximum(254)
    0x95, 0x01,       //     ReportCount(1)
    0x75, 0x08,       //     ReportSize(8)
    0x81, 0x02,       //     Input(Data, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)

    //     axis (1 byte)
    0x09, 0x34, //     UsageId(Ry[52])
    0x81, 0x02, //     Input(Data, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)

    //     axis (1 byte)
    0x09, 0x33, //     UsageId(Rx[51])
    0x81, 0x02, //     Input(Data, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)

    //     D-PAD (hat switch), (4 bits)
    0x09, 0x39,       //     UsageId(Hat Switch[57])
    0x46, 0x40, 0x01, //     PhysicalMaximum(320)
    0x65, 0x14,       //     Unit('degrees', EnglishRotation, Degrees:1)
    0x15, 0x01,       //     LogicalMinimum(1)
    0x25, 0x08,       //     LogicalMaximum(8)
    0x75, 0x04,       //     ReportSize(4)
    0x81, 0x02,       //     Input(Data, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)

    //     Feature notification (4 bits)
    0x09, 0x47, //     UsageId(Feature Notification[71])
    0x45, 0x00, //     PhysicalMaximum(0)
    0x65, 0x00, //     Unit(None)
    0x81, 0x02, //     Input(Data, Variable, Absolute, NoWrap, Linear, PreferredState, NoNullPosition, BitField)

    // ___ CAPABILITIES (FEATURE) REPORT ___
    0x09, 0x00,                     // USAGE (undefined)
    0x15, 0x00,                     // LogicalMinimum(0)
    0x25, 0xff,                     // LogicalMaximum(256)
    0x85, RID_FEATURE_CAPABILITIES, // REPORT ID
    0x75, 0x08,                     // Report Size (8)
    0x95, CAPABILITIES_REPORT_SIZE,     // Report count
    0xb1, 0x23,                     // FEATURE (Cnst,var,abs,Nprf)

    // ___ CONFIG (FEATURE) REPORT ___
    0x09, 0x00,               // USAGE (undefined)
    0x85, RID_FEATURE_CONFIG, // REPORT ID
    0x75, 0x08,               // Report Size (8)
    0x95, CONFIG_REPORT_SIZE, // Report count
    0xb1, 0xa2,               // FEATURE (Data,var,abs,Nprf,Vol)

    // END APPLICATION
    0xC0 // EndCollection()
};

#endif