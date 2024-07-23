# Unit test: game pad

**NOTE**:: This is also an integration test for modules `hidImplementation` and `userSettings`.

## Purpose and summary

To test that:

- the device works properly as a gamepad in a computer.
- the device can send and receive data by the means of HID feature reports.

This procedure is the same for all HID implementations, however, each implementation requires a different sketch:

- [NimBLE implementation](./NimBLEimplTest.ino)
- [ESP-Arduino BLE implementation](./../ESPBLEimplTest/ESPBLEimplTest.ino)
- [USB implementation](./../USBImplTest/USBImplTest.ino)

## Hardware setup

- BLE implementations: nothing required, except for an external antenna on some devices.
- USB implementation: an ESP32-S3 DevKit board is required (LillyGo T-QT was tested).

Output through USB serial port at 115200 bauds.

## Software setup

Computer:

- Windows 10 or later
- Bluetooth 4.2 or later (not required in USB implementation)
- Joystick testing software able to display 128 buttons. Note that Window's device property page is not suitable for this.
- SimpleHIDWrite.exe: available at [http://janaxelson.com/hidpage.htm](http://janaxelson.com/hidpage.htm). There is a modern clone at [https://github.com/Robmaister/SimplerHidWrite](https://github.com/Robmaister/SimplerHidWrite).

## Specific notes for USB implementation

Your DevKit must enter *boot loader* mode in order to upload the test sketch.
Reset while holding `GPIO #0` to low voltage. In LillyGo T-QT, press `reset` button while holding `IO0` button.
Use this board configuration in Arduino IDE:

- Board: "ESP32S3 Dev Module (esp32)" (or your actual board brand).
- USB-Mode: "USB-OTG (TinyUSB)".

## Specific notes for BLE implementation

- If the device is paired because of a previous test, unpair it first (delete from the bluetooth control panel).
- Before pairing, wait for the "Device ready" notification at the serial monitor.
- Will show as "NimBLEimplTest", "ESPBLEimplTest" (depending on which implementation is being tested).

## Procedure and expected output

- Ignore this output message while running this test: `(Waiting for connection)`.

### Auto power off

Not applicable to USB implementation.

1. Reset
2. Output must match:

   ```text
   --START--
   *** DISCOVERING ***
   --GO--
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
   --GO--
   ```

3. Before a minute elapses, pair and connect with the device using the Bluetooth controls in your computer.
4. Output must match:

   ```text
   *** CONNECTED ***
   ```

5. Open the joystick test application and keep it visible.
6. Rx, Ry and Rz axes should increase each second. At almost max value, must return to zero.

### Battery level

Not applicable to USB implementation.

1. Go to the Bluetooth page of the control panel. Look for the device.
2. Check battery level. Must show a decreasing number from 100% down to 50%, then up to 100% again.

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

### Device configuration (HID report)

1. Open "SimpleHidWriter.exe". Locate `Device VID=1D50` in the top area, and click on it.
2. You should see continuous report lines starting with `RD 01`. Ignore them. Click on `Clear` from time to time.
3. Enter `03` at field `ReportID`.
4. Enter `FF FF FF FF FF` at fields below `ReportID`.
5. Click on `Set Feature` , then on `Get Feature`.
6. Must show a line starting with: `RD 03  00 01 7F 42 01`.
7. Enter `01 FF 40 01 FF` at fields below `ReportID`.
8. Click on `Set Feature` , then on `Get Feature`.
9. Must show a line starting with: `RD 03  01 01 40 42 01`.
10. Serial output must show: `CMD: recalibrate axes`.

### Capabilities (HID report)

1. Enter `02` at field `ReportID`.
2. Enter `00 00 00 00 00 00 00 00` at fields below `ReportID`.
3. Click on `Set Feature` , then on `Get Feature`.
4. Must show the following line: `RD 02  51 BF xx xx xx xx 07 00 xx xx xx xx xx xx xx xx`. Ignore `xx`.

### Buttons map (HID report)

1. Enter `04` at field `ReportID`.
2. Enter `00 FF FF` at fields below `ReportID`.
3. Click on `Set Feature` , then on `Get Feature`.
4. Must show a line starting with: `RD 04  00 00 40`.
5. Enter `02 FF FF` at fields below `ReportID`.
6. Click on `Set Feature` , then on `Get Feature`.
7. Must show a line starting with: `RD 04  02 02 42`.
8. Enter `02 00 40` at fields below `ReportID`.
9. Click on `Set Feature` , then on `Get Feature`.
10. Must show a line starting with: `RD 04  02 00 40`.
11. Enter `AA 00 00` at fields below `ReportID`.
12. Click on `Set Feature` , then on `Get Feature`.
13. Must show a line again starting with: `RD 04  02 00 40`.

## Reconnect

Regression test for Issue #3. Not applicable to USB implementation.

1. Reset. Wait a few seconds.
2. Ensure the device is connected again to the host computer.
3. Reopen the joystick test application (close and run again).
4. Ensure the buttons test is running again (as described before).
