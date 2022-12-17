# Notes on Human Interface Device (HID) implementation

This project implements a HID device which may be used both through the USB and Bluetooth GATT protocol stacks.
Most HID details can be found at file [HID_definitions.h](../../src/include/HID_definitions.h)

## License and copyright concerns

### USB

Any HID device requires a vendor identifier ("VID" from now on) in order to work. Those identifiers are assigned to individual vendors through a license managed by ["The USB Implementers Forum"](https://www.usb.org/about), aka "USB-IF". That license is not open-source-friendly and the $6000 fee discourages any home-made project attempt.

More information at [Open source Hardware Association](https://www.oshwa.org/faq/#usb-vendor-id)

As a result, you are not allowed to use this project outside of your personal sphere. You are not allowed to exhibit the USB logo nor any other licensed material from the USB-IF.

Derivative works should obtain a different VID.

The VID is defined at constant `OPEN_SOURCE_VENDOR_ID`.

### Bluetooth

When using the Bluetooth stack, HID devices may use VIDs from different "sources". As seen at the [Device Information Service Specification](https://www.bluetooth.org/docman/handlers/downloaddoc.ashx?doc_id=244369):

| Vendor ID source | Description                                                                          |
| :--------------: | ------------------------------------------------------------------------------------ |
|       0x01       | Bluetooth SIG-assigned Device ID Vendor ID value from the Assigned Numbers document |
|       0x02       | USB Implementer’s Forum assigned Vendor ID value                                     |
|      other       | Reserved for future use                                                              |

I don't know what is the legal situation when a reserved VID source is used.

## HID descriptor

This project make use of a number of HID reports:

| Report ID |  Type   | Purpose                           |
| :-------: | :-----: | --------------------------------- |
|     1     |  Input  | Buttons, clutch, POV and the like |
|     2     | Feature | Wheel configuration               |
|     3     | Feature | Wheel capabilities                |

Note that feature reports are both read and write.

## Data format of report ID 1

- Button states: bytes indexed from 0 to 15. One bit per button (1=pressed, 0=non pressed). The least significant bit is the first button.
- Clutch: byte indexed 16. This is a signed byte in the range -127 to 127.
- POV (D-PAD): byte indexed 17. Valid values are in the range 0-8.

## Data format of report ID 2

While writing, any value outside of the valid range will be ignored, so they me be used to mask which fields to modify or not.

| Byte index | Valid values | Purpose (field)            | Note                                                  |
| :--------: | :----------: | -------------------------- | ----------------------------------------------------- |
|     0      |  0, 1 or 2   | Function of clutch paddles |                                                       |
|     1      |     any      | "ALT" buttons state        | non-zero means enabled, except for HEX 80 (see below) |
|     2      | -127 to 127  | Current bite point         | signed byte                                           |

_Other notes_:

- Any valid value written will be saved to flash memory.
- When byte index 1 is set to 80 (hexadecimal) in a write operation, the field will be ignored.
- The same goes for byte index 2, since 80 (hexadecimal) is -128 (signed decimal), outside of the valid range.


## Data format of report ID 3

This is an unsigned 32-bits (4 bytes) set of flags. Write attempts will be ignored, so they are read only. Flags are indexed starting from the least significant bit.

| Bit index | Meaning            |
| :-------: | ------------------ |
|     0     | Has clutch paddles |
|     1     | Has "ALT" buttons  |

Non listed bits are reserved for future use.
