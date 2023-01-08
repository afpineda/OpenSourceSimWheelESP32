# Notes on Human Interface Device (HID) implementation

This project implements a HID device (sorry for the redundancy) which may be used both through the USB and Bluetooth GATT protocol stacks. Most relevant implementation details can be found at file [HID_definitions.h](../../src/include/HID_definitions.h)

This document does not intent to explain HID devices. A good source is [Jan Axelson's Lakeview Research](http://www.janaxelson.com/hidpage.htm).

## License and copyright concerns

### USB

Any HID device requires a vendor identifier ("VID" from now on) in order to work. Those identifiers are assigned to individual vendors through a license managed by ["The USB Implementers Forum"](https://www.usb.org/about), aka "USB-IF". That license is not open-source-friendly and the $6000 fee discourages any home-made project attempt.

More information at [Open source Hardware Association](https://www.oshwa.org/faq/#usb-vendor-id)

As a result, you are not allowed to use this project outside of your personal sphere. You are not allowed to exhibit the USB logo nor any other licensed material from the USB-IF.

Derivative works should obtain a different VID.

**This project does not feature an USB implementation right now**. However, it is prepared for that.

### Bluetooth

When using the Bluetooth stack, HID devices may use VIDs from different "sources". As seen at the [Device Information Service Specification](https://www.bluetooth.org/docman/handlers/downloaddoc.ashx?doc_id=244369):

| Vendor ID source | Description                                                                         |
|:----------------:| ----------------------------------------------------------------------------------- |
| 0x01             | Bluetooth SIG-assigned Device ID Vendor ID value from the Assigned Numbers document |
| 0x02             | USB Implementer’s Forum assigned Vendor ID value                                    |
| other            | Reserved for future use                                                             |

I don't know what is the legal situation when a reserved VID source is used. At least, there is a way to avoid the USB-IF's licenses.

## HID reports

This project make use of a number of HID reports:

| Report ID | Type    | Purpose                           |
|:---------:|:-------:| --------------------------------- |
| 1         | Input   | Buttons, clutch, POV and the like |
| 2         | Feature | Wheel capabilities                |
| 3         | Feature | Wheel configuration               |

Note that feature reports are both read and write.

## Data format of report ID 1

| Field                | Bits size | Byte index |
| -------------------- |:---------:|:----------:|
| Buttons state        | 128       | 0          |
| Rz axis              | 8         | 16         |
| Ry axis              | 8         | 17         |
| Rx axis              | 8         | 18         |
| POV (D-PAD)          | 4         | 19         |
| Feature notification | 4         | 19         |

- Buttons state: one bit per button (1=pressed, 0=non-pressed). The least significant bit is the first button.
- Axes: a signed byte in the range 0 to 254.
  - Rz: F1-Style clutch (combined imput from left and right clutch paddles)
  - Ry: Left clutch paddle
  - Rx: Right clutch paddle
- POV (D-PAD): 4 least significant bits of byte index 19. Range: 0 to 8.
- Feature notification: 4 most significant bits of byte index 19. Valid values: 0 (nothing to notify) or 3 (wheel configuration has changed).

## Data format of report ID 2

Write attempts will be ignored, so this report is read only.

| Byte index | Size (Bytes) | Purpose (field) | Note                             |
|:----------:|:------------:| --------------- | -------------------------------- |
| 0          | 2            | Magic number    | Always set to BF51 (hexadecimal) |
| 2          | 2            | Major Version   | Version of this specification    |
| 4          | 2            | Minor Version   | Version of this specification    |
| 6          | 2            | Flags           | Device capabilities              |

Report ID 1 (input) is not affected by versioning.

### Magic number

The purpose of such a field is to enable device detection and recognition, without the need to rely on a particular VID or product ID.

### Major and minor version

Two different major versions means incompatible data formats. A greater minor version means a backwards-compatible data format.
Host-side software should check for version compatibility. Some examples:

| Data version | Supported version at host | Result       |
|:------------:|:-------------------------:|:------------:|
| 1.5          | 2.0                       | Incompatible |
| 2.0          | 1.1                       | Incompatible |
| 1.1          | 1.6                       | Incompatible |
| 2.7          | 2.3                       | Compatible   |

### Flags

The "flags" field is a set of 1-bit flags. Flags are indexed starting from the least significant bit. Non indexed bits are reserved for future use. Current flags are enumerated in `deviceCapability_t` at file [SimWheelTypes.h][def]

## Data format of report ID 3

While writing, any value outside of the valid range will be ignored, so they me be used to mask which fields to modify or not.

| Byte index | Size (bytes) | Purpose (field)                        |
|:----------:|:------------:| -------------------------------------- |
| 0          | 1            | Function of clutch paddles             |
| 1          | 1            | "ALT" buttons state                    |
| 2          | 1            | Current bite point                     |
| 3          | 1            | Simple command / Current battery level |

### Function of clutch paddles

Read/write.
Valid values are enumerated in `clutchFunction_t` at file [SimWheelTypes.h][def].
Write FF (hexadecimal) to ignore this field.

### "ALT" buttons state

Read/write.
Non zero means enabled.
Write FF (hexadecimal) to ignore this field.

### Current bite point

Read/write.
Write FF (hexadecimal) to ignore this field.

### Simple command / Current battery level

At read:

- Retrieve current battery level. Non meaningful if there is no battery. Check capabilities.

At write:

- Send a simple command. Valid commands are enumerated in `simpleCommands_t` at file [SimWheelTypes.h][def].
- Write FF (hexadecimal) to ignore this field.

[def]: ../../src/include/SimWheelTypes.h


