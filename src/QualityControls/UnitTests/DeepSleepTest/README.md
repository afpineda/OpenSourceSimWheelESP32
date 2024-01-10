# Unit test: deep sleep and wake up

## Purpose and summary

To test deep sleep and wake up.

## Hardware setup

Actual GPIO numbers are defined at [debugUtils.h](./debugUtils.h) and [DeepSleepTest.ino](./DeepSleepTest.ino).
Use this [test circuit](../../Protoboards/ProtoBoard-ESP32-Dekvit-C-1.diy):

![Test circuit image](../../Protoboards/ProtoBoard-ESP32-Dekvit-C-1.png)

We are using a single digital button in pull-up configuration, named "power button" for later reference:

- **ESP32C3 boards**: one end wired to `GPIO #2`.
- **Other boards**: one end wired to `TEST_ROTARY_SW` and  **externally** pulled up. The existing KY-040 rotary encoder do the work.

The other end wired to `GND` (ground).

Output through USB serial port at 115200 bauds.

## Procedure and expected output

1. Reset. Ignore all output.
2. Wait for the message:

   ```text
   Push POWER button to enter deep sleep mode...
   ```

3. Press and release the power button.
4. Output must match:

   ```text
   Entering deep sleep mode
   ```

5. Wait a few seconds. **No other output** must appear.
6. Press and release the power button again.
7. Ignore output from the operating system itself.
8. Output must match:

   ```text
   --START--
   Wake up caused by external signal using RTC_CNTL
   POWER ON and running
   ```

9. Ignore any other output.
