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

## Additional notes

- If your device uses USB connectivity, the display name will match your device name,
  which can be set in your custom firmware using the API call `hidImplementation::begin()`.

- If you have two or more BLE devices using firmware from this project,
  all of them will show the same display name, because they share the same hardware ID.
  If you want to set a different hardware ID for any of your devices, first, you must modify
  the source code:

  - Edit file ["HID_definitions.h"](../src/include/HID_definitions.h).
  - Locate the following line:

    ```c++
    #define BLE_PRODUCT_ID 0xffff
    ```

  - Set any other 16-bits hexadecimal value, right to "BLE_PRODUCT_ID". That is the new PID.
  - You may have to run the [sources setup procedure](./firmware/sourcesSetup_en.md) again.
  - Compile and upload your custom firmware [again](./hardware/DevKits_en.md).
