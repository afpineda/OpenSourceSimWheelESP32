# Changing your device's display name (Windows only) or Hardware ID (BLE only)

Once your BLE simwheel/button box is paired and connected to your host computer,
It will be available in the "Game Controllers" control panel.
However, it will show a generic display name like "4 axis 128 button device with hat switch".

> [!CAUTION]
> The following workaround will rename all devices sharing the same hardware ID,
> not just your device.

## Custom display name by means of the companion app (Windows only)

On Windows computers, the
[companion app](https://github.com/afpineda/SimWheelESP32Config)
will do the work for you. See the
[usage notes](https://github.com/afpineda/SimWheelESP32Config/blob/main/doc/UsageNotes_en.md).

## Custom display name by means of the Windows registry editor (Windows only)

1. Open the **"registry editor"** (regedit.exe) **with user privileges** (not administrator).
2. Navigate to the following folder (registry key):

   > KEY_CURRENT_USER\System\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM

3. Inside that folder, locate the subfolder (registry key) for your device.

   Those keys are named using this pattern: "**VID** -hex number- &**PID** -hex number- ",
   where "VID" means "vendor identifier" and "PID" means "product identifier".
   Together, they are a (plug-and-play) hardware identifier (ID).
   If your device uses **BLE connectivity** and the default hardware ID set in this project,
   that registry key should be **"VID_1D50&PID_FFFF"**.

4. Double-click (edit) the value "**OEMName**".
5. Set a custom display name and hit "enter".

This works for a single computer and user account.
If you need to propagate your custom display name to another user or computer:

- Right-click on the alluded registry key.
- Select "Export" to create a ".reg" file.
- Move the ".reg" file to another computer or account and double-click.

This works as a backup measure, too.

## Custom hardware ID (BLE only)

If you have two or more BLE devices using firmware from this project,
**all of them will show the same display name, because they share the same hardware ID**.
However, there are two (non-exclusive) ways to assign a different hardware ID to each device.

### By means of the companion app

A custom VID or PID can be stored by the firmware in flash memory to be used on the subsequent boot.
Use the [companion app](https://github.com/afpineda/SimWheelESP32Config).

### In source code

- Locate the following API call (not to be taken literally):

  ```c++
  hid::configure(
        DEVICE_NAME,
        DEVICE_MANUFACTURER,
        ...);
  ```

- Add two parameters to the right:
  - a non-zero 16 bits number as a custom VID.
  - a non-zero 16 bits number as a custom PID.
  Numbers 0 and FFFF (hexadecimal) are reserved and must not be used.

For example, [Setup1.ino](../src/Firmware/Setup1/Setup1.ino) shows:

```c++
hid::configure(
   DEVICE_NAME,
   DEVICE_MANUFACTURER,
   false);
```

In order to set the custom VID 16 and the custom PID 32,
substitute with:

```c++
hid::configure(
   DEVICE_NAME,
   DEVICE_MANUFACTURER,
   false,
   16,
   32);
```

For convenience, [CustomSetup.ino](../src/Firmware/CustomSetup/CustomSetup.ino)
do the work in advance.
Uncomment the following lines:

```c++
// #define BLE_CUSTOM_VID <here>
// #define BLE_CUSTOM_PID <here>
```

And replace `<here>` with your custom numbers.
For example (equivalent to the previous example):

```c++
#define BLE_CUSTOM_VID 16
#define BLE_CUSTOM_PID 32
```

## USB connectivity

If your device uses USB connectivity, the display name will match your device name,
which can be set in your custom firmware using the API call `hid::configure()`.
Any custom VID/PID will be ignored since USB uses the hardware ID
licensed to your DevKit's manufacturer.

## Why all this mess?

Believe it or not, the HID specification does not include a "display name".
Windows figures out what to show in this manner:

- *USB*: the display name is taken from the "product string",
  which is a certain [USB string descriptor](https://beyondlogic.org/usbnutshell/usb5.shtml#StringDescriptors).

- *Bluetooth classic*: the display name is taken from the value of the SDP attribute called "Service Name".
  See table 5.3 (section 5.3.3) from the
  [Bluetooth HID specification](https://www.bluetooth.com/specifications/specs/human-interface-device-profile-1-1-1/).

- *BLE*: according to the
  [HID over GATT specification](https://www.bluetooth.com/specifications/specs/hid-over-gatt-profile-1-0/)
  there is nowhere to get a display name.
  When a device is advertised, Windows ought to take note of the device name, but it doesn't.
  The open-source community is still in the dark about how commercial products show a custom display name.
