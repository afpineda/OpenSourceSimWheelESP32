# Unit test: deep sleep and wakeup

## Purpose and summary

To test deep sleep and wake up.

## Harware setup

Actual GPIO pins are defined at [debugUtils.h](debugUtils.h)

Single digital button using internal pull-up:

- One wire connected to `GND`.
- Other wire connected to `TEST_POWER_PIN`.
- Named "power button" for later reference.

Output through USB serial port at 115200 bauds.

## Procedure and expected output

1. Reset. Ignore all output.
2. Wait for the message:
   
   ```
   Push POWER button to enter deep sleep mode...
   ```
3. Press and release the power button.
4. Output must match:
   
   ```
   Entering deep sleep mode
   ```
5. Wait a few seconds. **No other output** must appear.
6. Press and release the power button again.
7. Ignore output from the operating system itself.
8. Output must match:
   
   ```
   --START--
   Wakeup caused by external signal using RTC_CNTL
   POWER ON and running
   ```
9. Ignore any other output.