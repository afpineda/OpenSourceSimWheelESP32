# Unit test: battery monitor

## Purpose and summary

To test that the battery monitor daemon runs in the
background taking ADC readings of battery voltage.

## Hardware setup

Use this [test circuit](../../../Protoboards/MainTestBoard.diy):

![Test circuit image](../../../Protoboards/MainTestBoard.png)

We are using the battery monitor subsystem, only:
`TEST_BATTERY_READ_ENABLE` and `TEST_BATTERY_READ`.

`Battery(+)` will be wired to different inputs through this procedure.
Use a wire with Dupond terminals.
Get ready a fully charged battery. Do not wire the positive terminal yet.

> [!CAUTION]
> Do not wire the negative pole of the battery (**risk of fire**).
> Wire `GND` from the powerboost module instead.

Output through USB serial port at 115200 bauds.

## Procedure and expected output

1. Ensure `Battery(+)` is not wired.
2. Reset. Ignore output from the operating system itself.
3. Output must match:

   ```text
   --READY--
   SoC: 100
   --GO--
   SoC: 66
   ```

4. Wait for 20 seconds. No output should appear.
5. Wire `battery(+)` to `GND`.
6. Wait for 20 seconds. No output should appear.
7. Ignore repeated messages from now on.
8. Wire `battery(+)` to `3V3`.
9. Wait for 10 seconds.
10. Take note of the displayed number, named "L1" for future reference.
11. Wire `battery(+)` to the positive terminal of the battery.
12. Wait for 10 seconds.
13. Take note of the displayed number, named "L2".
14. "L2" must be greater then "L1".
