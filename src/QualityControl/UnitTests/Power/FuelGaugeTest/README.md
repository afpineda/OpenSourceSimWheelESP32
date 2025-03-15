# Unit test: fuel gauge

## Purpose and summary

To test that the battery monitor daemon runs in the background asking the fuel gauge for state of charge.

## Hardware setup

**Get ready a fully charged battery**.

Wire a MAX17043 module as described in the
[battery monitor subsystem](../../../../../doc/hardware/subsystems/BatteryMonitor/BatteryMonitor_en.md),
including the battery itself.
However, this test will get power from the USB cable alone.

Output at 115200 bauds over the USB serial port.

## Procedure

1. Reset. Ignore output from the operating system itself.
2. Output must match:

   ```text
   --READY--
   --GO--
   ```

3. Wait for **ten minutes** or so. It takes time for the fuel gauge to profile the battery.
4. Last output line show (not to be taken literally):

   ```text
   SoC: 86%
   ```

   or any other percentage. Take note of that percentage.

5. Unplug the battery from the circuit.
6. Output must show (literally):

   ```text
   SoC: 66%
   ```

7. Plug the battery to the circuit again.
8. Last output line must show another percentage **very close** to the first percentage (above or below). For example:

   ```text
   SoC: 89%
   ```

9. Wait for two minutes.
10. No further output must show.
    However, a new percentage may appear, 1% lesser than the previous one.
    That is OK, too.
