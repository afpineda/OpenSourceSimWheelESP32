# Unit test: bluetooth game pad

## Purpose and summary

To test if the device works properly as a wireless game pad in a computer.

## Hardware setup

Nothing required, except for an external antenna on some devices.
Output through USB serial port at 115200 bauds.

## Software setup

Computer:

- Windows 10 or later
- Bluetooth 4.2 or later
- Joystick testing software from Planet's Pointy ( [http://www.planetpointy.co.uk/joystick-test-application/](http://www.planetpointy.co.uk/joystick-test-application/) ) or any other able to display 128 buttons. Note that Window's device property page is not suitable for this.
- SimpleHIDWrite.exe: available at [http://janaxelson.com/hidpage.htm](http://janaxelson.com/hidpage.htm). There is a modern clone at [https://github.com/Robmaister/SimplerHidWrite](https://github.com/Robmaister/SimplerHidWrite).

Mobile device:

- "nRF Connect for Mobile" by Nordic Semiconductors:
  - Android: [https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp&gl=US](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp&gl=US)
  - Apple: [https://apps.apple.com/us/app/nrf-connect-for-mobile/id1054362403](https://apps.apple.com/us/app/nrf-connect-for-mobile/id1054362403)

## Procedure and expected output

_Notes_:

- Ignore this output message while running this test: `(Waiting for connection)`
- At pairing, wait for the "Device ready" notification.

### Part 1: auto power off

If the device is paired because of a previous test, unpair it first (delete from the bluetooth control panel). Will show as "SimWheelTest".

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

### Part 2: clutch value

1. Open the joystick test application and keep it visible.
2. Reset.
3. Output must match:

   ```text
   --START--
   *** DISCOVERING ***
   --GO--
   ```

4. Before a minute elapses, pair and connect with the device using the bluetooth controls in your computer.
5. Output must match:

   ```text
   *** CONNECTED ***
   ```

6. Rx, Ry and Rz axes should increase each second. At almost max value, should return to zero.

### Part 3: buttons

1. Open the joystick test application and keep it visible.
2. Reset.
3. Output must match:

   ```text
   --START--
   *** DISCOVERING ***
   --GO--
   ```

4. Before a minute elapses, pair and connect with the device using the bluetooth controls in your computer.
5. Output must match:

   ```text
   *** CONNECTED ***
   ```

6. Buttons should be pressed and released every second. Pressed buttons must follow this timed pattern:
   - First, buttons in the range 1-64
   - Then, buttons in the range 65-128
   - Back to first
7. Pressed buttons also follows a binary pattern starting from 0. So check this sequence at the very connection time (pressed button numbers):
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
8. Restart this test if something is missed.
9. Point-of-view control (aka "Hat switch" or "POV") must follow this pattern in a loop:
   - None pressed
   - Up
   - Up-right
   - Right
   - Down-right
   - Down
   - Down-left
   - Left
   - Up-left

### Part 4: battery level

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

5. Battery level must match 100%.
6. Battery level should decrease 1% every second. At 50%, will reset to 100% and continue decreasing.
   Note: Battery level may not update at the computer side.

### Part 5: NuS protocol

1. Go to your mobile device.
2. Turn bluetooth and location on.
3. Open nRF Connect.
4. Unpair the system from your computer.
5. Before a minute elapses:
    1. Click "SCAN" (nRF connect)
    2. Locate the system being tested. _Appearance: [964] Gamepad (HID subtype)_
    3. Connect
6. If more than one minute elapses, you will be unable to locate the system and connect. In such a case, reset the board and repeat the previous step.
7. Locate the "Nordic UART Service" and click.
8. Locate the "RX Characteristic" and click on the arrow icon.
9. Type the following text and send: `testing`.
10. The following line must appear at the serial monitor:

    ```text
    COMMAND: testing
    ```

### Part 6: device configuration HID report

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

5. Open "SimpleHidWriter.exe". Locate `Device VID=501D` in the top area, and click on it.
6. You should see continuous report lines starting with `RD 01`. Ignore them. Click on `Clear` from time to time.
7. Enter `03` at field `ReportID`.
8. Enter `FF FF FF` at fields below `ReportID`.
9. Click on `Set Feature` , then on `Get Feature`.
10. Must show a line starting with: `RD 03  00 01 FE`.
11. Enter `01 00 80` at fields below `ReportID`.
12. Click on `Set Feature` , then on `Get Feature`.
13. Must show a line starting with: `RD 03  01 00 80`.

### Part 7: capabilities HID report

1. Enter `02` at field `ReportID`.
2. Enter `00 00 00 00 00 00 00 00` at fields below `ReportID`.
3. Click on `Set Feature` , then on `Get Feature`.
4. Must show the following line: `RD 02 51 BF xx xx xx xx 07 00`. Ignore `xx`.
