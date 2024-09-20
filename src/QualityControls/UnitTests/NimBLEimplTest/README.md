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
- SimpleHIDWrite.exe: available at [http://janaxelson.com/hidpage.htm](http://janaxelson.com/hidpage.htm).
  There is a modern clone at [https://github.com/Robmaister/SimplerHidWrite](https://github.com/Robmaister/SimplerHidWrite).

## Specific notes for USB implementation

Your DevKit must enter *boot loader* mode in order to upload the test sketch.
Reset while holding `GPIO #0` to low voltage. In LillyGo T-QT, press `reset` button while holding `IO0` button.
Use this board configuration in Arduino IDE:

- Board: "ESP32S3 Dev Module (esp32)" (or your actual board brand).
- USB-Mode:
  - DevKit with just one USB port: "USB-OTG (TinyUSB)".
  - DevKit with two USB ports: "Hardware CDC and JTAG".
- USB CDC On Boot: Disabled.

## Specific notes for BLE implementation

- If the device is paired because of a previous test, unpair it first (delete from the Bluetooth control panel).
- Before pairing, wait for the "Device ready" notification at the serial monitor.
- Will show as "NimBLEimplTest" or "ESPBLEimplTest" (depending on which implementation is being tested).

## Procedure and expected output

- There is no serial output in USB devices. Ignore that part.
- The expected factory VID in USB devices is 0x303A
  ([Espressif](https://docs.espressif.com/projects/esp-iot-solution/en/latest/usb/usb_overview/usb_vid_pid.html)).
- Ignore this output message while running this test: `(Waiting for connection)`.

### Auto power off

Not applicable to USB implementation.

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

1. Open "SimpleHidWriter.exe". Locate this test device in the top area, and click on it.
   - *BLE implementation*: look for `Device VID= ... PID= ...`.
      The proper values where printed at boot `(Actual VID / PID)`.
   - *USB implementation*: look for the device name.
2. You should see continuous report lines starting with `RD 01`. Ignore them. Click on `Clear` from time to time.
3. Enter `03` at field `ReportID`.
4. Enter `FF FF FF FF FF FF` (6 times `FF`) at fields below `ReportID`.
5. Click on `Set Feature` , then on `Get Feature`.
6. Must show a line starting with `RD 03  00 01 7F 42 01 00`.
7. Enter `01 FF 40 01 FF 01` at fields below `ReportID`.
8. Click on `Set Feature` , then on `Get Feature`.
9. Must show a line starting with: `RD 03  01 01 40 42 01 00`.
10. Serial output must show: `CMD: recalibrate axes`.
11. Enter `FF FF FF 06 FF FF` at fields below `ReportID`.
12. Click on `Set Feature`.
13. Serial output must show: `CMD: reverse right axis`.

### Capabilities (HID report)

1. Enter `02` at field `ReportID`.
2. Enter `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00` (16 times `00`) at fields below `ReportID`.
3. Click on `Set Feature` , then on `Get Feature`.
4. Must show the following line: `RD 02  51 BF xx xx xx xx 07 00 xx xx xx xx xx xx xx xx`.
   Ignore `xx`.

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

### UI control (HID report)

1. Enter `06` at field `ReportID`.
2. Click on `Get Feature`.
3. Must show a line starting with: `RD 06  01`.
4. Enter `00 00` at fields below `ReportID`.
5. Click on `Set Feature`.
6. Serial output must show: `UI control: next page`.
7. Enter `00 01` at fields below `ReportID`.
8. Click on `Set Feature`.
9. Serial output must show: `UI control: previous page`.

## Telemetry data

1. Enter `14` at field `ReportID`.
2. Enter `4b 5e 06 63 01 00 40 01` at fields below `ReportID`.
3. Click on `Set Report`.
4. Serial output must show this line among others:
   `powertrain: K 1630 99 1 0 320`
5. Enter `15` at field `ReportID`.
6. Enter `00 02 ff 00 01 00 07 40 fe` at fields below `ReportID`.
7. Click on `Set Report`.
8. Serial output must show this line among others:
   `ecu: 0 1 1 0 1 0 7 64 100`
9. Enter `16` at field `ReportID`.
10. Enter `01 00 02 00 03 00 04 e2 09 30 31 31 30 32 33` at fields below `ReportID`.
11. Serial output must show this line among others:
    `ecu: 1 0 1 0 1 0 1 0 2530 01:10:23`
12. Enter `17` at field `ReportID`.
13. Enter `57 85 00 1c 27 22 01 40 1f 01 90 19 b8 22 31 30 30 31 33 32` at fields below `ReportID`.
14. Serial output must show this line among others:
    `gauges: 87 1.33 100.12 2.90 80.0 1 65.44 8888 10:01:32`

### Custom Hardware ID (HID report)

Not applicable to USB implementation.

**Note**:

- You must complete this sub-test from start to end.
- In case of failure, use [UserSettingsTest](../UserSettingsTest/README.md) to
  issue a "clear NVS flash memory" command.
- If Windows refuses to unpair the test device, restart your computer first.
- Take care not to type wrong numbers!

Procedure:

1. Enter `05` at field `ReportID`.
2. Enter `5E 04 8E 02 24 28` at fields below `ReportID`.
3. Click on `Set Feature` , then on `Get Feature`.
4. Must show a line starting with `RD 05  5E 04 8E 02 00 00`.
   Ignore `xx`.
5. Close "SimpleHidWriter.exe".
6. Unpair the test device using the Bluetooth control panel.
7. Reset. Take note of the contents of the message `Factory default VID / PID: ...`.
8. At some point, serial output must show `Actual VID / PID: 045e / 028e`.
9. Pair and connect the test device to the host computer.
10. Open the "Game controllers" control panel (Windows only).
    Must show a device called "Controller (XBOX 360 for Windows)".
11. Open "SimpleHidWriter.exe". Locate this test device in the top area, and click on it.
    This time, look for `Device VID=045E PID=028E`.
12. Enter `05` at field `ReportID`.
13. Enter `00 00 00 00 96 AA` at fields below `ReportID`.
14. Click on `Set Feature` , then on `Get Feature`.
15. Must show a line starting with `RD 05  yy yy zz zz 00 00`.
    `yy yy` must match the factory default VID **in reverse order** of bytes.
    `zz zz` must match the factory default PID in reverse order of bytes.
16. Enter `00 00 00 00 00 00` at fields below `ReportID`.
17. Must show the **same line** as before, starting with `RD 05`.
18. Close "SimpleHidWriter.exe".
19. Unpair the test device using the Bluetooth control panel.
20. Reset.
21. The message `Actual VID / PID: ...` must show the same numbers as `Factory default VID / PID: ...`.
22. Pair and connect the test device to the host computer.

## Reconnect

Regression test for Issue #3. Not applicable to USB implementation.

1. Reset. Wait a few seconds.
2. Ensure the device is connected again to the host computer.
3. Reopen the joystick test application (close and run again).
4. Ensure the buttons test is running again (as described before).
