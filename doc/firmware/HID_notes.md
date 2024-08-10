# Notes on Human Interface Device (HID) implementation

This project implements a HID device (sorry for the redundancy)
which may be used both through the USB and Bluetooth GATT protocol stacks.
Most relevant implementation details can be found at file
[HID_definitions.h](../../src/include/HID_definitions.h)

This document does not intent to explain HID devices.
A good source is [Jan Axelson's Lakeview Research](http://www.janaxelson.com/hidpage.htm).

## License and copyright concerns

### USB

Any HID device requires a vendor identifier ("VID" from now on) in order to work.
Those identifiers are assigned to individual vendors through a license managed by
["The USB Implementers Forum"](https://www.usb.org/about), aka "USB-IF".
That license is not open-source-friendly and the $6000 fee discourages any home-made project attempt.

More information at [Open source Hardware Association](https://www.oshwa.org/faq/#usb-vendor-id).

This project features an USB implementation using the existing PID (product identifier)
and VID from your DevKit manufacturer, which you should not change.

As a result, you are not allowed to use this project outside of your personal sphere.
You are not allowed to exhibit the USB logo nor any other licensed material from the USB-IF.

### Bluetooth

When using the Bluetooth stack, HID devices may use VIDs from different "sources".
As seen at the
[Device Information Service Specification](https://www.bluetooth.org/docman/handlers/downloaddoc.ashx?doc_id=244369):

| Vendor ID source | Description                                                                         |
| :--------------: | ----------------------------------------------------------------------------------- |
|       0x01       | Bluetooth SIG-assigned Device ID Vendor ID value from the Assigned Numbers document |
|       0x02       | USB Implementer’s Forum assigned Vendor ID value                                    |
|      other       | Reserved for future use                                                             |

I don't know what is the legal situation when a reserved VID source is used.
At least, there is a way to avoid the USB-IF's licenses.

## HID reports

This project make use of a number of HID reports:

| Report ID |  Type   | Purpose                           |
| :-------: | :-----: | --------------------------------- |
|     1     |  Input  | Buttons, clutch, POV and the like |
|     2     | Feature | Wheel capabilities                |
|     3     | Feature | Wheel configuration               |
|     4     | Feature | User-defined buttons map          |
|     5     | Feature | Custom hardware ID                |

Note that feature reports are both read and write.

## Data format of report ID 1 (input)

| Field                | Bits size | Byte index |
| -------------------- | :-------: | :--------: |
| Buttons state        |    128    |     0      |
| Rz axis              |     8     |     16     |
| Ry axis              |     8     |     17     |
| Rx axis              |     8     |     18     |
| POV (D-PAD)          |     4     |     19     |
| Feature notification |     4     |     19     |

- Buttons state: one bit per button (1=pressed, 0=non-pressed).
  The least significant bit is the first button.
- Axes: an unsigned byte in the range 0 to 254.
  - Rz: F1-Style clutch (combined input from left and right clutch paddles)
  - Ry: Left clutch paddle
  - Rx: Right clutch paddle
- POV (D-PAD): 4 least significant bits of byte index 19. Range: 0 to 8.
- Feature notification: 4 most significant bits of byte index 19.
  Valid values: 0 (nothing to notify) or 3 (wheel configuration has changed).

## Data format of report ID 2 (wheel capabilities)

Write attempts will be ignored, so this report is read-only.

| Byte index | Size (Bytes) | Purpose (field) | Note                             | Since data version |
| :--------: | :----------: | --------------- | -------------------------------- | ------------------ |
|     0      |      2       | Magic number    | Always set to BF51 (hexadecimal) | 1.0                |
|     2      |      2       | Major Version   | Version of this specification    | 1.0                |
|     4      |      2       | Minor Version   | Version of this specification    | 1.0                |
|     6      |      2       | Flags           | Device capabilities              | 1.0                |
|     8      |      8       | ID              | Chip identifier                  | 1.1                |

Report ID 1 (input) is not affected by versioning.

### Magic number

The purpose of such a field is to enable device detection and recognition,
without the need to rely on a particular VID or product ID.

### Major and minor version

Two different major versions means incompatible data formats.
A greater minor version means a backwards-compatible data format.
Host-side software should check for version compatibility.
Some examples:

| Data version | Supported version at host |    Result    |
| :----------: | :-----------------------: | :----------: |
|     1.5      |            2.0            | Incompatible |
|     2.0      |            1.1            | Incompatible |
|     1.1      |            1.6            | Incompatible |
|     2.7      |            2.3            |  Compatible  |

Current data version is 1.2.

### Flags

The "flags" field is a set of 1-bit flags.
Flags are indexed starting from the least significant bit.
Non indexed bits are reserved for future use.
Current flags are enumerated in `deviceCapability_t` at file [SimWheelTypes.h][def]

### ID

This is the internal chip identifier as reported by
[esp_efuse_mac_get_default()](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/misc_system_api.html).
Useful to distinguish one device from another.

## Data format of report ID 3 (wheel configuration)

While writing, any value outside of the valid range will be ignored,
so they me be used to mask which fields to modify or not.

| Byte index | Size (bytes) | Purpose (field)                        | Since data version |
| :--------: | :----------: | -------------------------------------- | ------------------ |
|     0      |      1       | Working mode of clutch paddles         | 1.0                |
|     1      |      1       | Working mode of ALT buttons            | 1.0                |
|     2      |      1       | Current bite point                     | 1.0                |
|     3      |      1       | Simple command / Current battery level | 1.0                |
|     4      |      1       | Working mode of DPAD inputs            | 1.1                |
|     5      |      1       | Security lock                          | 1.2                |

### Working mode of clutch paddles

Read/write (unless locked).
Valid values are enumerated in `clutchFunction_t` at file [SimWheelTypes.h][def].
Write FF (hexadecimal) to ignore this field.

### Working mode of ALT buttons

Read/write (unless locked).
Non zero means "ALT mode". Zero means "regular buttons".
Write FF (hexadecimal) to ignore this field.

### Current bite point

Read/write (unless locked).
Write FF (hexadecimal) to ignore this field.

### Simple command / Current battery level

At read:

- Retrieve current battery level. Non meaningful if there is no battery. Check capabilities.

At write (unless locked):

- Send a simple command. Valid commands are enumerated in `simpleCommands_t` at file [SimWheelTypes.h][def].
  This is a summary:

  | Simple command (decimal) | Description                                                                        |
  | ------------------------ | ---------------------------------------------------------------------------------- |
  | 1                        | Recalibrate analog clutch paddles (if any).                                        |
  | 2                        | Restart battery auto-calibration. Ignored if the battery was "factory-calibrated". |
  | 3                        | Reset user-defined button mapping to factory defaults.                             |
  | 4                        | Save all user settings at once (including button mapping).                         |
  | 5                        | Reverse left clutch paddle polarity (analog only if any).                          |
  | 6                        | Reverse right clutch paddle polarity (analog only if any).                         |

- Write FF (hexadecimal) to ignore this field.

### Working mode of DPAD inputs

Read/write (unless locked).
Non zero means "navigation controls". Zero means "regular buttons".
Write FF (hexadecimal) to ignore this field.

### Security lock

**Read-only**.

When zero, write is allowed. Otherwise, write is forbidden.

For security concerns, the user can lock or unlock all writing attempts to HID reports by using just hardware inputs.
This is a security precaution to stop unauthorized configuration modifications caused by rogue programs.

## Data format of report ID 4 (user-defined buttons map)

| Byte index | Size (bytes) | Purpose (field)                                        | Since data version |
| :--------: | :----------: | ------------------------------------------------------ | ------------------ |
|     0      |      1       | Selected firmware-defined button number                | 1.1                |
|     1      |      1       | User-defined button number when ALT mode is disengaged | 1.1                |
|     2      |      1       | User-defined button number when ALT mode is engaged    | 1.1                |

**Important note**: any change in the user-defined map is **not** automatically saved to flash memory.
In order to save, you must issue the corresponding *simple command* using report ID 3.

### Firmware-defined button number

At read:

- A selected button number in the range from 0 to 63, which identifies a firmware-defined button.
- Any value outside of the previous range may be read, which means no button number is selected.

In order to select another button number, write to this field first.

At write (unless locked):

- Select a firmware-defined button number. Any value outside the valid range will be ignored.
- You should read after a write in order to known the user-defined map for the selected button number.
  Note that another application may also write this field, so don't assume the button you select is the button you will read next.

### User-defined map (bytes at index 1 and 2)

At read:

- The current user-defined button number for the selected firmware-defined button number.
- A value of FF (hexadecimal) means the selected button is not available.

At write (unless locked):

- Any value in the range from 0 to 127: set a user-defined button number for the selected firmware-defined button number.
- Any value outside of the previous range: just select another firmware-defined button number, but do not overwrite current map.

Examples (pseudo-code):

- To list all available inputs and current map:

  ```c++
  for (uint8_t i = 0; i < 64; i++) {
     report4.write(i,0xFF,0xFF);
     report4.read(j,map,mapAlt);
     if (i != j)
        another_app_is_interfering();
     else if ((map > 127) || (mapAlt > 127))
        input_number_not_available(i);
     else
        show_map(i,map,mapAlt);
  }
  ```

- To map button number 13 to button numbers 16 (ALT disengaged) and 112 (ALT engaged):

  ```c++
  report4.write(13,16,112);
  ```

- To map button number 17 as it was in factory defaults:

  ```c++
  report4.write(17,17,17+64);
  ```

- To save current map to flash memory:

  ```c++
  report3.write(0xFF,0xFF,0xFF,4);
  ```

- To revert user-defined map to factory defaults (and save):

  ```c++
  report3.write(0xFF,0xFF,0xFF,3);
  report3.write(0xFF,0xFF,0xFF,4);
  ```

## Data format of report ID 5 (custom hardware ID)

| Byte index | Size (bytes) | Purpose (field)   | Since data version |
| :--------: | :----------: | ----------------- | ------------------ |
|     0      |      2       | Custom vendor ID  | 1.2                |
|     2      |      2       | Custom product ID | 1.2                |
|     4      |      2       | Control code      | 1.2                |

### Custom vendor ID (VID) and product ID (PID)

Those fields enable the user to set a custom VID and PID for BLE devices only.
Changes will be available after a reset or power-off.

At read:

- *USB devices*: zero is read at both fields.
- *BLE devices*: The PID and VID configured for the subsequent reboot will be retrieved.

At write (unless locked):

- *USB devices*: both fields are ignored.
- *BLE devices*:
  - If both VID and PID are set to 0000, the hardware ID will return to factory defaults after the next reboot.
  - Otherwise, the device will save the given VID and PID in flash memory and use them after the next reboot.

### Control code

This field prevents accidental changes.

At read:

- No meaning

At write:

- Ignored in USB devices.
- If either VID or PID (or both) is set to 0000, this field must match AA96 (hexadecimal).
- Otherwise, this field must match $(VID*PID)\mod{65536}$.

  For example, if $VID=7504$ and $PID=303$, then this control code must match
  $(7504*303)\mod{65536}=2273712\mod{65536}=45488$

**No changes are made if there is no match.**

[def]: ../../src/include/SimWheelTypes.h
