# Unit test: USB game pad

## Purpose and summary

To test that:

- the device works properly as a USB-wired gamepad in a computer.
- the device can send and receive data by the means of HID feature reports.

## Hardware setup

An ESP32-S3 DevKit board is required (LillyGo T-QT was used in this test).

## Software setup

Computer:

- Windows 10 or later
- Joystick testing software from Planet's Pointy ( [http://www.planetpointy.co.uk/joystick-test-application/](http://www.planetpointy.co.uk/joystick-test-application/) ) or any other able to display 128 buttons. Note that Window's device property page is not suitable for this.
- SimpleHIDWrite.exe: available at [http://janaxelson.com/hidpage.htm](http://janaxelson.com/hidpage.htm). There is a modern clone at [https://github.com/Robmaister/SimplerHidWrite](https://github.com/Robmaister/SimplerHidWrite).

## Procedure and expected output

_Notes_:

Your DevKit must enter *bootloader* mode in order to upload the test sketch.
Reset while holding `GPIO #0` to low voltage. In LillyGo T-QT, press `reset` button while holding `I00` button.
Use this board configuration in Arduino IDE:

- Board: "ESP32S3 Dev Module (esp32)" (or your actual board brand).
- USB-Mode: "USB-OTG (TinyUSB)".

### Analog Axes (left clutch, right clutch and combined clutch)

1. Open the joystick test application and keep it visible.
2. Rx, Ry and Rz axes should increase each second. At almost max value, must return to zero.

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

1. Open "SimpleHidWriter.exe". Locate your DevKit in the top area, and click on it. Should be shown as "USBImplTest".
2. You should see continuous report lines starting with `RD 01`. Ignore them. Click on `Clear` from time to time.
3. Enter `03` at field `ReportID`.
4. Enter `FF FF FF FF` at fields below `ReportID`.
5. Click on `Set Feature` , then on `Get Feature`.
6. Must show a line starting with: `RD 03  00 01 7F 42`.
7. Enter `01 FF 40 01` at fields below `ReportID`.
8. Click on `Set Feature` , then on `Get Feature`.
9. Must show a line starting with: `RD 03  01 01 40 42`.

### Capabilities (HID report)

1. Enter `02` at field `ReportID`.
2. Enter `00 00 00 00 00 00 00 00` at fields below `ReportID`.
3. Click on `Set Feature` , then on `Get Feature`.
4. Must show the following line: `RD 02  51 BF xx xx xx xx 07 00`. Ignore `xx`.
