# Unit test: battery monitor

## Purpose and summary

To test that the battery monitor runs in the background taking ADC readings of battery voltage.

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
   --GO--
   ```

3. A message must appear every 5 seconds. For example (not to be taken literally):

   ```text
   Reading: 1695
   ```

4. Ignore repeated messages from now on.
5. Wire `battery(+)` to `GND`.
6. **No output** should appear.
7. Wire `battery(+)` to `3V3`.
8. Take note of the displayed number, named "L1" for future reference.
9. Wire `battery(+)` to the positive terminal of the battery.
10. Take note of the displayed number, named "L2".
11. "L2" must be greater then "L1".