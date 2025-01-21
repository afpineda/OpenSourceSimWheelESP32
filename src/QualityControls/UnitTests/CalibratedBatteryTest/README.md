# Unit test: battery calibration algorithm

## Purpose and summary

We aim to verify the battery calibration algorithm's ability to convert ADC voltage readings into a battery level.

## Hardware setup

Actual GPIO numbers are defined at [debugUtils.h](./debugUtils.h).
Use this [test circuit](../../Protoboards/TestBoard1.diy):

![Test circuit image](../../Protoboards/TestBoard1.png)

We are using the battery monitor subsystem, only: `TEST_BATTERY_READ_ENABLE` and `TEST_BATTERY_READ`.

`Battery(+)` will be wired to different inputs through this procedure. Use a wire with Dupond terminals.
Get ready a fully charged battery. Do not wire the positive terminal yet.

> [!CAUTION]
> Do not wire the negative pole of the battery (**risk of fire**).
> Wire `GND` from the powerboost module instead.

Output through USB serial port at 115200 bauds.

## Other pre-requisites

Run the [battery calibration procedure](../../../Firmware/BatteryTools/BatteryCalibration/README.md)
(once) or [restore](../../../Firmware/BatteryTools/RestoreBatteryCalibration/README.md)
the battery calibration data.

If there is no calibration data in flash memory, you will get a warning.
However, this test may run just the same,
but the auto-calibrated algorithm will be tested instead.

## Procedure and expected output

1. Reset. Ignore output from the operating system itself.
2. Output must match:

   ```text
   --READY--
   --GO--
   ```

3. Wire `battery(+)` to `GND`.
4. Output must show: `SoC: 66%`.
5. Wire `battery(+)` to `3V3`.
6. Output must show (not to be taken literally): `SoC: 3%`,
   or any other percentage close to 0%.
7. Wire `battery(+)` to the positive pole of the battery.
8. Output must show (not to be taken literally): `SoC: 91%`,
   or any other percentage close to 100%.
