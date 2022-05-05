# Integration test: User-to-PC event propagation

## Purpose and summary

To test that system events are propagated as joystick/gamepad input events at the PC side.

## Hardware setup

Actual GPIO numbers are defined at [debugUtils.h](./debugUtils.h).

Rotary encoder:

- KY-040 type (with external pull-up resistors)
- `CLK` pin attached to `TEST_ROTARY_CLK`
- `DT` pin attached to `TEST_ROTARY_DT`
- `SW` pin attached to `TEST_ROTARY_SW`. 

Button Matrix (6 buttons): same setup as the [Button Matrix unit test](../../UnitTests/ButtonMatrixTest/README.md).
An external antenna may be required at some devices.

This test is designed for 128x64 pixels with a 132x64 display controller. When using another setup, the sketch must be tweaked.  OLED must be powered and wired to the corresponding `SCL` and `SCA`pins in the DevKit board.

For later reference:

- "ALT" is the built-in push button into the rotary encoder
- "CLUTCH1" is the button numbered 1 in the button matrix
- "CLUTCH2" is the button numbered 2 in the button matrix
- "MENU" is the button numbered 3 in the button matrix
- "BTN4" is the button numbered 4 in the button matrix
- "BTN5" is the button numbered 5 in the button matrix
- "BTN6" is the button numbered 6 in the button matrix
- "RCW" means rotary's clockwise rotation
- "RCCW" means rotary's counter-clockwise rotation

## Software setup (computer)

- Windows 10 or later
- Bluetooth 4.0 or later
- Joystick testing software from Planet's Pointy ( [http://www.planetpointy.co.uk/joystick-test-application/](http://www.planetpointy.co.uk/joystick-test-application/) ) or any other able to display 128 buttons. Note that Window's device property page is not suitable for this.

## Procedure and expected output

1. Ensure the system is paired and connected to the computer.
2. Press and release each button quickly, one by one. Test event registration at the joystick testing software:
   1. "CLUTCH1": A joystick axis should move from zero to some point near the middle.
   2. "CLUTCH2": Same as "CLUTCH1".
   3. "ALT": No event should be registered.
   4. "MENU": Button #3 should be registered.
   5. "BTN4", "BTN5" and "BTN6": Buttons #4, #5 and #6 should be registered, respectively.
3. Press and hold "ALT", then press and release "BTN4", then release "ALT". Button #68 should be registered.
4. Rotate "RCW". Button #8 should be registered.
5. Rotate "RRCW". Button #9 should be registered.
6. Press and hold "BTN5", then press and hold "BTN6", then release both. Buttons #5 and #6 should be registered at the same time.
7. Press and hold "MENU", wait 3 seconds, then release.
8. Rotate "RCW", then "RRCW". No event should be registered.
9. Press and release "BTN6". No event should be registered.