# Unit test: game pad

## Purpose and summary

To test that:

- the device works properly as a gamepad in a computer.
- the device can send and receive data by the means of HID feature reports.

This procedure is the same for all HID implementations,
however, each implementation requires a different sketch:

- [NimBLE implementation](./NimBLEimplTest/NimBLEimplTest.ino)
- [ESP-Arduino BLE implementation](./ESPBLEimplTest/ESPBLEimplTest.ino)
- [USB implementation](./USBImplTest/USBImplTest.ino)

## Hardware setup

- BLE implementations: nothing required, except for an external antenna on some devices.
- USB implementation: an ESP32-S3 DevKit board is required
  (LillyGo T-QT and ESP32S3-DevKit-C were tested).

Output through USB serial port at 115200 bauds.

## Software setup

Computer:

- Windows 10 or later
- Bluetooth 4.2 or later (not required in USB implementation)
- Joystick testing software able to display 128 buttons.
  Note that Window's device property page is not suitable for this.
- SimpleHIDWrite.exe:
  available at [http://janaxelson.com/hidpage.htm](http://janaxelson.com/hidpage.htm).
  There is a modern clone at
  [https://github.com/Robmaister/SimplerHidWrite](https://github.com/Robmaister/SimplerHidWrite).

## Specific notes for the USB implementation

In LillyGo T-QT, press the `reset` button while holding the `IO0` button
to enter *bootloader mode*.

Use this board configuration in Arduino IDE:

- Board: "ESP32S3 Dev Module (esp32)" (or your actual board brand).
- USB-Mode:
  - DevKit with just one USB port: "USB-OTG (TinyUSB)".
  - DevKit with two USB ports: "Hardware CDC and JTAG".
- USB CDC On Boot: Disabled.

## Specific notes for the BLE implementation

- If the device is paired because of a previous test,
  unpair it first (delete from the Bluetooth control panel).
- Before pairing, wait for the `*** DISCOVERING ***` notification at the serial monitor.
- It will be shown as "NimBLEimplTest", "ESPBLEimplTest" or "ESPBLEimplTest"
  (depending on which implementation is being tested).

## Procedure and expected output

- There is no serial output in some ESP32S3 devkit boards
  when using the USB implementation.
  Ignore that part unless you have dual USB ports in your DevKit.
  In such a case, attach two USB cables: one for serial output
  and another for the HID implementation.
- The expected factory VID in USB devices is 0x303A
  ([Espressif](https://docs.espressif.com/projects/esp-iot-solution/en/latest/usb/usb_overview/usb_vid_pid.html)).
- Ignore this output message while running this test:
  `(Waiting for connection)`.

### Auto power off

Not applicable to the USB implementation.

1. Reset
2. Output must match:

   ```text
   --START--
   *** DISCOVERING ***
   ```

3. Wait for a minute or so.
4. Output must match:

   ```text
   *** POWER OFF ***
   (Reset required)
   ```

### Analog Axes (left clutch, right clutch and combined clutch)

1. Reset.
2. Output must match:

   ```text
   --START--
   *** DISCOVERING ***
   ```

3. Before a minute elapses, pair and connect with the device using the Bluetooth controls in your computer.
4. Output must match:

   ```text
   *** CONNECTED ***
   ```

5. Open the joystick test application and keep it visible.
6. Rx, Ry and Rz axes should increase each second. At almost max value, must return to zero.

### Battery level

Not applicable to the USB implementation.

1. Go to the Bluetooth page of the control panel.
   Look for the device.
2. Check battery level.
   Must show a decreasing number from 100% down to 50%,
   then up to 100% again.

### Buttons

1. Buttons should be pressed and released every second.
   If buttons are numbered starting with #1, pressed buttons must follow this timed pattern :
   - Buttons #1 and #65 are pressed at the same time.
   - Previous buttons are released and buttons #2 and #66 are pressed at the same time.
   - Previous buttons are released and buttons #3 and #67 are pressed at the same time.
   - The pattern continues until buttons #64 and #128 are pressed.
   - Then, the pattern starts again.
2. Restart this test if something is missed.
3. Point-of-view control (aka "Hat switch" or "POV") must follow this pattern in a loop:
   - Not pressed
   - Up
   - Up-right
   - Right
   - Down-right
   - Down
   - Down-left
   - Left
   - Up-left

### HID reports

Open "SimpleHidWriter.exe".
Locate this test device in the top area, and click on it:

- *BLE implementation*: look for `Device VID= ... PID= ...`.
  The proper values were printed at boot `(Actual VID / PID)`.
- *USB implementation*: look for the device name.

You should see continuous report lines starting with `RD 01`.
Ignore them. Click on `Clear` from time to time.

**Just check there are no error messages** in *SimpleHidWriter*.
The behaviour of these HID reports is not part of this tests.
That is the subject of another automated test.

1. Enter `02` at field `ReportID`.
2. Click on `Set Feature`, then on `Get Feature`.
3. Enter `03` at field `ReportID`.
4. Click on `Set Feature` , then on `Get Feature`.
5. Enter `04` at field `ReportID`.
6. Click on `Set Feature` , then on `Get Feature`.
7. Enter `05` at field `ReportID`.
8. Click on `Set Feature` , then on `Get Feature`.
9. **Note**: do not confuse `Set Report` with `Set Feature`.
10. Enter `14` at field `ReportID`.
11. Click on `Set Report`.
12. Enter `15` at field `ReportID`.
13. Click on `Set Report`.
14. Enter `16` at field `ReportID`.
15. Click on `Set Report`.
16. Enter `17` at field `ReportID`.
17. Click on `Set Report`.
18. Enter `1E` at field `ReportID`.
19. Click on `Set Report`.

### Reconnect

Regression test for Issue #3.
Not applicable to the USB implementation.

1. Reset. Wait a few seconds.
2. Ensure the device is connected again to the host computer.
   Look for the message `*** CONNECTED ***` at the serial monitor.
3. Reopen the joystick test application (close and run again).
4. Ensure the buttons test is running again (as described before).
