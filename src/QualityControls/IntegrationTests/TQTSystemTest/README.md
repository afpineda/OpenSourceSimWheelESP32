# BLE System test for the LilyGO T-QT devkit board

This is a *big-bang* system test for the [LilyGO T-QT devkit board](https://github.com/Xinyuan-LilyGO/T-QT).
The purpose is to test firmware compatibility with this board.

## Hardware setup

No circuit is involved here, just the devkit board and the USB cable. Onboard "IO0" and "KEY" switches are the only input hardware. Built-in battery monitor is also enabled.

## Software setup (computer)

- Windows 10 or later
- Bluetooth 4.2 or later
- Joystick testing software from Planet's Pointy ( [http://www.planetpointy.co.uk/joystick-test-application/](http://www.planetpointy.co.uk/joystick-test-application/) ) or Window's device property page.

## Firmware upload

At Arduino IDE, configure the board manager as this:

- Board: "ESP32S3Dev"
- USB CDC on boot: "Enabled".

In order to enter "boot mode" click the reset button while holding "IO0".

## Procedure and expected output

1. Open the serial monitor (at 115200 bauds).
2. Reset.
3. Check there are no error messages.
4. Ensure the system is paired and connected to the computer. It should appear as "TQTSystemTestBLE" to the hosting computer.
5. Open the joystick test app.
6. Press and hold the "IO0" switch. Check button number #1 is "on".
7. Release the "IO0" switch. Check no button is "on".
8. Press and hold "KEY" switch. Check button number #2 is "on".
9. Release the "KEY" switch. Check no button is "on".
