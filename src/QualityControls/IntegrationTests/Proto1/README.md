# Integration test: User-to-PC event propagation

## Purpose and summary

To test that system events are propagated as joystick/gamepad input events at the PC side.

## Hardware setup

Actual GPIO numbers are defined at [debugUtils.h](./debugUtils.h).
Use this [test circuit](../../Protoboards/ESP32-WROOM-DevKitC-1.diy):

![Test circuit image](../../Protoboards/ProtoBoard-ESP32-Dekvit-C-1.png)

For later reference:

- "ALT" is the built-in push button into the rotary encoder.
- "CLUTCH1" and "CLUTCH2" are the potentiometers as shown in the protoboard.
- "BTN**N**" is the button numbered "**N**" in the button matrix as shown in the protoboard.
- "RCW" means rotary's clockwise rotation.
- "RCCW" means rotary's counter-clockwise rotation.

## Software setup (computer)

- Windows 10 or later
- Bluetooth 4.2 or later
- Joystick testing software from Planet's Pointy ( [http://www.planetpointy.co.uk/joystick-test-application/](http://www.planetpointy.co.uk/joystick-test-application/) ) or any other able to display 128 buttons. Note that Window's device property page is not suitable for this.
- SimpleHIDWrite.exe: available at [http://janaxelson.com/hidpage.htm](http://janaxelson.com/hidpage.htm). There is a modern clone at [https://github.com/Robmaister/SimplerHidWrite](https://github.com/Robmaister/SimplerHidWrite).

## Procedure and expected output

### Part 1

1. Ensure the system is paired and connected to the computer.
2. Press and release these buttons quickly, one by one. Test event registration at the joystick testing software:
   1. "CLUTCH1": Axis Rz should move from zero to some point near the middle.
   2. "CLUTCH2": Same as "CLUTCH1".
   3. "ALT": No event should be registered.
   4. "BTN2": should be registered as #3.
   5. "BTN3": should be registered as D-PAD up.
   6. "BTN4": should be registered as D-PAD down.
   7. "BTN5": should be registered as D-PAD left.
   8. "BTN6": should be registered as D-PAD left.
   9. "BTN7": should be registered as #8.
3. Press and hold "ALT", then press and release "BTN2", then release "ALT". Button #67 should be registered.
4. Rotate "RCW". Button #13 should be registered.
5. Rotate "RCCW". Button #14 should be registered.
6. Press and hold "BTN2", then press and hold "BTN7", then release both. Buttons #3 and #8 should be registered at the same time.
7. Press and hold "BTN2", then press and hold "BTN3", then release both.
8. Press and release "ALT". Button #11 should be registered.
9. Press and hold "BTN2", then press and hold "BTN4", then release both.
10. Axis Rz should go to zero.
11. Move "CLUTCH1". Axis Rx must reflect the movement.
12. Move "CLUTCH2". Axis Ry must reflect the movement.
13. Press and hold "BTN2", then press and hold "BTN5", then release both.
14. Press and release "BTN3". Button #4 should be registered.

### Part 2

1. Open "SimpleHidWriter.exe". Locate `Device VID=501D` in the top area, and click on it.
2. You should see continuous report lines starting with `RD 01`. Ignore them. Click on `Clear` from time to time.
3. Enter `04` at field `ReportID`.
4. Enter `07 09 0A` at fields below `ReportID`. Click on `Set Feature`.
5. Enter `03` at field `ReportID`.
6. Enter `FF 01 FF FF 01` at fields below `ReportID`. Click on `Set Feature`.
7. Now return to the joystick testing software.
8. Press and release "BTN7". Should be registered as button #10.
9. Press and hold "ALT", then press and release "BTN7", then release "ALT". Button #11 should be registered.
10. Press and release "BTN3". Should be registered as as D-PAD up.
