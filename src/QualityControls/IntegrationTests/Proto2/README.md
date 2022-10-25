# Integration test: power and battery

## Purpose and summary

To test battery level and power off from the config menu.

## Hardware setup

Actual GPIO numbers are defined at [debugUtils.h](./debugUtils.h).

The [battery monitor circuit](../../../../doc/hardware/subsystems/BatteryMonitor/BatteryMonitor_en.md) must be in place. 

- Wire `battEN` to GPIO `TEST_BATTERY_READ_ENABLE`
- Wire `battREAD` to GPIO `TEST_BATTERY_READ`
- `Battery(+)` wired to the positive terminal of a fully charged battery (3.7V or more). 
- Wire `3V3` and `GND` as usual.
- Wire the negative terminal of the battery to GND.

Rotary encoder:

- KY-040 type (with external pull-up resistors)
- `CLK` pin attached to `TEST_ROTARY_CLK`
- `DT` pin attached to `TEST_ROTARY_DT`
- `SW` pin attached to `TEST_ROTARY_SW`

Button Matrix (6 buttons): same setup as the [Button Matrix unit test](../../UnitTests/ButtonMatrixTest/README.md).
An external antenna may be required at some devices.

This test is designed for 128x64 pixels with a 132x64 display controller. When using another setup, the sketch must be tweaked.  OLED must be powered and wired to the corresponding `SCL` and `SCA` pins in the DevKit board.

For later reference:

- "ALT" is the built-in push button into the rotary encoder.
- "MENU" is the button numbered 3 in the button matrix
- "RCW" means rotary's clockwise rotation
- "RCCW" means rotary's counter-clockwise rotation

## Software setup (computer)

- Windows 10 or later
- Bluetooth 4.0 or later

## Procedure and expected output

1. Reset. Wait for the "welcome" screen to disappear.
2. Pair and connect.
3. Press and hold "MENU" for 3 seconds, then release.
4. Go to "Battery" menu. 
5. Go to "Recalibrate" and click "ALT". 
6. Wait for 3 minutes.
7. Go to "Battery" menu again. Show battery level.
8. Battery level must be 100%.
9. Go to "Power off" menu and click "ALT".
10. The system should enter deep sleep.
11. Unwire `Battery(+)`.
12. Click "ALT". The "welcome" screen must show up.
13. Go to "Battery" menu again. Show battery level.
14. Battery level must be 66%.