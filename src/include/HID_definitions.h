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

// Report ID's
#define RID_INPUT_GAMEPAD 0x01
#define RID_FEATURE_CONFIG 0x02
#define RID_FEATURE_CAPABILITIES 0x03

// Report sizes (bytes)
#define GAMEPAD_REPORT_SIZE 18
#define CONFIG_REPORT_SIZE 2
#define FEATURES_REPORT_SIZE 4

// GAME CONTROLLER APPEARANCES
#define CONTROLLER_TYPE_GAMEPAD 0x05
#define CONTROLLER_TYPE_JOYSTICK 0x04

// INPUT REPORT constants
#define BUTTON_COUNT 128

// PNP identifiers
// DO NOT CHANGE. See:
// http://wiki.openmoko.org/wiki/USB_Product_IDs

#define OPEN_SOURCE_VENDOR_ID 0x1d50
#define PROJECT_PRODUCT_ID 0xffff

// Hardware revision
#define PRODUCT_REVISION 0x01

// HID report descriptor
static const uint8_t hid_descriptor[] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, CONTROLLER_TYPE_GAMEPAD, // USAGE

    0xa1, 0x01,            // COLLECTION (Application)
    0x85, RID_INPUT_GAMEPAD, // REPORT ID
    0x55, 0x00,            //   UNIT_EXPONENT (0)
    0x65, 0x00,            //   UNIT (None)

    // __ Buttons __ (16 bytes=128 bits)
    0x05, 0x09,         //   USAGE_PAGE (Button)
    0x19, 0x01,         //   USAGE_MINIMUM (Button 1)
    0x29, BUTTON_COUNT, //   USAGE_MAXIMUM
    0x15, 0x00,         //   LOGICAL_MINIMUM (0)
    0x25, 0x01,         //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,         //   REPORT_SIZE (1)
    0x95, BUTTON_COUNT, //   REPORT_COUNT
    0x81, 0x02,         //   INPUT (Data,Var,Abs)

    // __ Clutch __ (1 byte)
    0x05, 0x02, //   USAGE_PAGE (Simulation Controls)
    0x09, 0xBB, // USAGE (Throttle)
    // 0x09, 0xC6, // USAGE (Clutch)
    0x15, 0x81, // LOGICAL_MINIMUM (-127)
    0x25, 0x7F, // LOGICAL MAXIMUM (127)
    0x75, 0x08, // REPORT_SIZE (8)
    0x95, 0x01, // REPORT_COUNT (1)
    0x81, 0x02, // INPUT (Data,Var,Abs)

    // __ Hat swith (DPAD) __ (1 byte)
    0xA1, 0x00,       // COLLECTION (Physical)
    0x05, 0x01,       // USAGE_PAGE (Generic Desktop)
    0x09, 0x39,       // USAGE (Hat Switch)
    0x15, 0x01,       // Logical Min (1)
    0x25, 0x08,       // Logical Max (8)
    0x35, 0x00,       // Physical Min (0)
    0x46, 0x3B, 0x01, // Physical Max (315)
    0x65, 0x12,       // Unit (SI Rot : Ang Pos)
    0x81, 0x42,       // Input (Data, Variable, Absolute)
    0xc0,             // END_COLLECTION (Physical)

    0x09, 0x00,                     // USAGE (undefined)
    0x85, RID_FEATURE_CONFIG, // REPORT ID
    0x75, 0x08,                     // Report Size (8)
    0x95, 0x03,                     // Report count (3)
    0xb1, 0xa2,                     // FEATURE (Data,var,abs,Nprf,Vol)

    // __ Feature: Wheel capabilities (read only) __ (4 byte)
    0x09, 0x00,                           // USAGE (undefined)
    0x85, RID_FEATURE_CAPABILITIES, // REPORT ID
    0x75, 0x20,                           // Report Size (32)
    0x95, 0x01,                           // Report count (1)
    0xb1, 0x23,                           // FEATURE (Cnst,var,abs,Nprf)

    0xc0 // END_COLLECTION (Application)
};

#endif