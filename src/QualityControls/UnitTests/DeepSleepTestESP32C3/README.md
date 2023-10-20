# Unit test: deep sleep and wake up

## Purpose and summary

To test deep sleep and wake up **on ESP32C3** boards, which expose a slightly different power management API than other ESP32 boards.

## Hardware setup

A single push button is required in pull-up configuration. One end wired to `GPIO #2`, the other end wired to `GND` (ground). Named "power button" for later reference.

Output through USB serial port at 115200 bauds.

## Procedure and expected output

Note that this procedure does not differ from [DeepSleepTest](../DeepSleepTest/README.md). Differs in the hardware being tested.

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
   Wakeup caused by external signal using RTC_CNTL
   POWER ON and running
   ```

9. Ignore any other output.
