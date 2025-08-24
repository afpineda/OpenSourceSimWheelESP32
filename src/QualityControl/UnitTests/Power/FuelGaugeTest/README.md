# Unit test: fuel gauge

## Purpose and summary

To test that the fuel gauge works:

- Battery presence is detected.
- Battery charging is detected.
- A SoC estimation is reported.

## Hardware setup

**Get ready a fully charged battery**.

Wire a MAX17043 module as described in the
[battery monitor subsystem](../../../../../doc/hardware/subsystems/BatteryMonitor/BatteryMonitor_en.md),
including the battery itself.
However, this test will get power from the USB cable alone.
Do not attach the battery to a powerboost module during this test.
If the battery is charging, you will get altered results,
thus invalidating this procedure.

Output at 115200 bauds over the USB serial port.

## Procedure

Ignore repeated output during this procedure.

1. Make sure the battery is not connected to the fuel gauge.
2. Reset. Ignore output from the operating system itself.
3. Wait for output. Must show:

   ```text
   Getting battery status...
   Battery presence: false
   Charging: false
   Wired power: true
   SoC: unknown
   Done.
   ```

4. Attach `Battery(+)` from the fuel gauge to the `3V3` output.
5. Wait for output. Must show (not literally):

   ```text
   Getting battery status...
   Battery presence: true
   Charging: false
   Wired power: unknown
   SoC: 0
   Done.
   ```

6. The number next to `SoC:` may be different, but close.
7. Attach the battery to the fuel gauge.
8. Wait for 20 seconds ignoring output.
9. Wait for output. Must show (not literally):

   ```text
   Getting battery status...
   Battery presence: true
   Charging: false
   Wired power: unknown
   SoC: 66
   Done.
   ```

10. The number next to `SoC:` must be anything not close to zero.
    Note: this number is not meaningful as it takes up to 10 minutes
    to profile the battery.
11. Detach the battery from the fuel gauge.
12. Wire `Battery(+)` to `5V`.
13. Wait for output. Must show (literally):

   ```text
   Getting battery status...
   Battery presence: unknown
   Charging: true
   Wired power: true
   SoC: unknown
   Done.
   ```
