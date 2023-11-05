# Unit test: bluetooth game pad

## Purpose and summary

To test that:

- the device works properly as a wireless game pad in a computer, **using the native ESP32's BLE stack**.
- the device can send and receive data by the means of HID feature reports.

## Hardware setup

Nothing required, except for an external antenna on some devices.
Output through USB serial port at 115200 bauds.

## Software setup

Computer:

- Windows 10 or later
- Bluetooth 4.2 or later
- Joystick testing software from Planet's Pointy ( [http://www.planetpointy.co.uk/joystick-test-application/](http://www.planetpointy.co.uk/joystick-test-application/) ) or any other able to display 128 buttons. Note that Window's device property page is not suitable for this.
- SimpleHIDWrite.exe: available at [http://janaxelson.com/hidpage.htm](http://janaxelson.com/hidpage.htm). There is a modern clone at [https://github.com/Robmaister/SimplerHidWrite](https://github.com/Robmaister/SimplerHidWrite).

## Procedure and expected output

_Notes_:

- Ignore this output message while running this test: `(Waiting for connection)`
- At pairing, wait for the "Device ready" notification.
- If the device is paired because of a previous test, unpair it first (delete from the bluetooth control panel). Will show as "SimWheelTest".

### Auto power off

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

3. Before a minute elapses, pair and connect with the device using the bluetooth controls in your computer.
4. Output must match:

   ```text
   *** CONNECTED ***
   ```

5. Open the joystick test application and keep it visible.
6. Rx, Ry and Rz axes should increase each second. At almost max value, must return to zero.

### Battery level

1. Go to the bluetooth page of the control panel. Look for the device.
2. Check battery level. Must show a decreasing number from 100% down to 50%, then up to 100% again.

### Buttons

1. Buttons should be pressed and released every second. Pressed buttons must follow this timed pattern:
   - First, buttons in the range 1-64
   - Then, buttons in the range 65-128
   - Back to first

2. Pressed buttons also follows a binary pattern starting from 0. So check this sequence at the very connection time (pressed button numbers):
   - None
   - 64
   - 2
   - 64,65
   - 3
   - 64,66
   - 2,3
   - 64,65,66
   - 4
   - 64,67
   - 2,4
   - 64,65,67
   - ...
3. Restart this test if something is missed.
4. Point-of-view control (aka "Hat switch" or "POV") must follow this pattern in a loop:
   - None pressed
   - Up
   - Up-right
   - Right
   - Down-right
   - Down
   - Down-left
   - Left
   - Up-left

### Device configuration (HID report)

1. Open "SimpleHidWriter.exe". Locate `Device VID=501D` in the top area, and click on it.
2. You should see continuous report lines starting with `RD 01`. Ignore them. Click on `Clear` from time to time.
3. Enter `03` at field `ReportID`.
4. Enter `FF FF FF FF` at fields below `ReportID`.
5. Click on `Set Feature` , then on `Get Feature`.
6. Must show a line starting with: `RD 03  00 01 7F 42`.
7. Enter `01 FF 40 01` at fields below `ReportID`.
8. Click on `Set Feature` , then on `Get Feature`.
9. Must show a line starting with: `RD 03  01 01 40 42`.
10. Serial output must show: `CMD: recalibrate axes`.

### Capabilities (HID report)

1. Enter `02` at field `ReportID`.
2. Enter `00 00 00 00 00 00 00 00` at fields below `ReportID`.
3. Click on `Set Feature` , then on `Get Feature`.
4. Must show the following line: `RD 02  51 BF xx xx xx xx 07 00`. Ignore `xx`.

## Reconnect

(Regression test for Issue #3)

1. Reset. Wait a few seconds.
2. Ensure the device is connected again to the host computer.
3. Reopen the joystick test application (close and ).
4. Ensure the buttons test is running again (as described before).
