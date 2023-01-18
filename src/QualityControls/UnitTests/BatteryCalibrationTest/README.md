# Unit test: battery calibration

## Purpose and summary

To test that:

- Battery calibration data is properly acquired and stored in flash memory.
- ADC readings maps properly into battery levels (0%-100%) according to calibration data.
- Auto-calibration works as expected.

## Hardware setup

Nothing required.
Output through USB serial port at 115200 bauds.

## Procedure and expected output

1. Reset. Ignore output from the operating system itself.
2. Output must match:

   ```text
   **TEST #1
   **TEST #2
   **TEST #3
   **TEST #4
   **TEST #5
   **TEST #6
   **TEST #7
   **TEST #8
   **TEST #9
   **TEST #10
   **TEST #11
   **TEST #12
   --END--
   ```

Notes:

- Prior calibration data, if any, will be erased from flash memory.
