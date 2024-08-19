# Unit test: fuel gauge

## Purpose and summary

To test that the battery monitor daemon runs in the background asking the fuel gauge for state of charge.

## Hardware setup

**Get ready a fully charged battery**.

Wire a MAX17043 module as described in the
[battery monitor subsystem](../../../../doc/hardware/subsystems/BatteryMonitor/BatteryMonitor_en.md),
including the battery itself.
However, this test will get power from the USB cable alone.

Additionally, prepare a rather simple battery discharge circuit using
just a 100 ohm resistor (or any other low impedance resistor).

Output at 115200 bauds over the USB serial port.

## Procedure

1. Reset. Ignore output from the operating system itself.
2. Output must match:

   ```text
   --READY--
   --GO--
   ```

3. Output must show (not to be taken literally):

   ```text
   SoC: 100%
   ```

   or any other percentage close to 100%. Take note of that percentage.

4. Unplug the battery from the circuit.
5. Output must show (literally):

   ```text
   SoC: 66%
   ```

6. Wire the battery to the discharge circuit. Wait for 5 to 10 minutes or so.
7. Unwire the battery from the discharge circuit.
8. Wire the battery to the test circuit again.
9. Output must show another percentage **below** the first percentage. For example:

   ```text
   SoC: 99%
   ```
