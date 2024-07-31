# Changing your device's display name (Windows and BLE connectivity only)

Once your BLE simwheel/button box is paired and connected to your host computer,
It will be available in the "Game Controllers" control panel.
However, it will show a generic display name like "4 axis 128 button device with hat switch".
In order to change that display name, follow this procedure.

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

## Custom display name for two or more open-source sim-wheels or button boxes

If you have two or more BLE devices using firmware from this project,
**all of them will show the same display name, because they share the same hardware ID**.
However, you may set a different hardware ID for each device by modifying the source code:

- Locate the following API call (not to be taken literally):

  ```c++
  hidImplementation::begin(
        DEVICE_NAME,
        DEVICE_MANUFACTURER,
        ...);
  ```

- Add a new parameter to the right: a non-zero 16 bits number as a custom PID.

For example, [Setup1.ino](../src/Firmware/Setup1/Setup1.ino) shows:

```c++
hidImplementation::begin(
   DEVICE_NAME,
   DEVICE_MANUFACTURER,
   false);
```

In order to set the custom PID number 16, substitute with:

```c++
hidImplementation::begin(
   DEVICE_NAME,
   DEVICE_MANUFACTURER,
   false,
   16);
```

## USB connectivity

If your device uses USB connectivity, the display name will match your device name,
which can be set in your custom firmware using the API call `hidImplementation::begin()`.
Any custom PID will be ignored since USB uses the hardware ID
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
  The open-source community is still in the dark about how commercial products display a custom display name.
