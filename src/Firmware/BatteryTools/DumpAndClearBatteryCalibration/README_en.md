# Dump and (optionally) erase battery calibration data

## Purpose

Use this procedure to dump current battery calibration data into the serial port and, optionally, erase it.

## Procedure

1. Upload the [sketch](./DumpAndClearBatteryCalibration.ino).
2. Open the serial monitor **quickly**.
3. Reset.
4. Current calibration data will be displayed.
5. After a countdown of 60 seconds, such data will be erased. If that is not your intention, unplug the USB cable.
