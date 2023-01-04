# Integration test: battery auto-calibration

## Purpose and summary

To test reading of battery voltage and conversion into a charge level.

## Hardware setup

Actual GPIO numbers are defined at [debugUtils.h](./debugUtils.h).
Use this [test circuit](../../Protoboards/ESP32-WROOM-DevKitC-1.diy):

![Test circuit image](../../Protoboards/ProtoBoard-ESP32-Dekvit-C-1.png)

We are using the battery monitor subsystem, only: `TEST_BATTERY_READ_ENABLE` and `TEST_BATTERY_READ`.

`Battery(+)` will be wired to different inputs through this procedure. Use a wire with Dupond terminals.
Get ready a fully charged battery. Wire the negative terminal to `GND`. Do not wire the positive terminal yet.

Output through USB serial port at 115200 bauds.

## Procedure and expected output

1. Reset. Ignore output from the operating system itself.
2. Output must match:

   ```text
   -- READY --
   -- GO --
   Last Battery level: 66
   ```

3. A message may appear every 5 seconds. For example (not to be taken literally):

   ```text
   Last Battery level: 99
   ```

4. Ignore unexpected messages from now on.
5. Keep `battery(+)` _unwired_. Next output must be:

   ```text
   Last Battery level: 66
   ```

6. Wire `battery(+)` to "3V3". Next output must be:

   ```text
   Last Battery level: 100
   ```

7. Keep `battery(+)` _unwired_ again. Next output must be:

   ```text
   Last Battery level: 66
   ```

8. Wire `battery(+)` to the positive terminal of the battery. Next output must be:

   ```text
   Last Battery level: 100
   ```

9. Wire `battery(+)`  to "3V3. Next output must be 6 or lesser. For example (not to be taken literally):

   ```text
   Last Battery level: 0
   ```

10. At this point, the board may enter deep sleep mode, thus freezing further output. This is the expected behavior.
