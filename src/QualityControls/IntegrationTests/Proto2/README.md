# Integration test: power, battery and digital clutch paddles

## Purpose and summary

To test battery level (calibrated) and power off from the config menu.

## Hardware setup

Actual GPIO numbers are defined at [debugUtils.h](./debugUtils.h).
Use this [test circuit](../../Protoboards/TestBoard1.diy):

![Test circuit image](../../Protoboards/TestBoard1.png)

**We are not using the potentiometers**. For later reference (this differs from previous tests):

- "CLUTCH1" is the button numbered #2 in the protoboard.
- "CLUTCH2" is the button numbered #3 in the protoboard.
- "Cycle CF" is the combination of buttons #7 and #6 in the protoboard.
- "RSW" is the built-in push button of the rotary encoder.
- "RCW" means rotary's clockwise rotation.
- "RCCW" means rotary's counter-clockwise rotation.

A powerboost module and a battery is needed. However, the test circuit does not require power from the battery (that is up to you).
Wire `battery(+)` and `POWERBOOST_GND`, at least.

## Software setup (computer)

- Windows 10 or later
- Bluetooth 4.2 or later
- Joystick testing software able to display 128 buttons. Note that Window's device property page is not suitable for this.

## Before start

- Follow the [battery calibration procedure](../../../Firmware/BatteryTools/BatteryCalibration/README.md).
- Make sure the device is not paired to the hosting PC. This includes previous tests.

## Procedure and expected output

### Power off/on

1. Keep the Bluetooth control panel visible.
2. Reset.
3. Go to "Add devices" for device discovery.
4. Make sure "Proto 2" shows up, but **do not connect**.
5. Wait for a minute or so.
6. "Proto 2" must disappear (the device is in deep sleep).
7. Push and release "RSW" (for wake up).
8. "Proto 2" must appear again.
9. Select and connect to "Proto 2" .

### Digital clutch paddle

1. Open the joystick test app.
2. At this point, the working mode of clutch paddles could be set to anything due to previous test. This is ok.
3. Hit "Cycle CF" 4 times and test "CLUTCH1" and "CLUTCH2" each time, in no particular order. In clutch mode, test "RCW" and "RCCW" for bite point calibration.
4. Hit "Cycle CF" until analog axis mode is selected (check using "CLUTCH1").
5. Wait for 30 seconds or so.
6. Reset. Close the joystick test app. Wait for the device to connect again.
7. Open the joystick test app again.
8. "CLUTCH1" must be configured in axis mode. Check.
9. Hit "Cycle CF" until "regular buttons" mode is selected (check using "CLUTCH1").
10. Wait for 30 seconds or so.
11. Reset. Close the joystick test app. Wait for the device to connect again.
12. "CLUTCH1" must be configured in "regular buttons" mode. Check.

### Battery level

1. Leave `battery(+)` unwired. Wait for 1 minute.
2. In the control panel, battery level must show "66%" for "Proto 2".
3. Wire `Battery(+)` to the battery positive pole.
4. Wait for 1 minute. Battery level must show anything but "66%", depending on your battery charge.
5. Wait for 5 to 30 minutes (depending on battery capacity). Whatever the battery level was, it must decrease at least 1%.
6. Wire `Battery(+)` to `3V3` in the protoboard.
7. The device may go to deep sleep. Check if the device is disconnected at the control panel. However, if this does not happen, the battery level must show a very low percentage after a minute or so.
