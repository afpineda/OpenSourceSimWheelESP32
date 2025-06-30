# Integration test: decoding of rotary coded switches

## Purpose and summary

To test that rotary coded switches are decoded to the input number assigned to their positions
using the `inputHub` namespace and any kind of input hardware.

## Hardware setup

Nothing special is required since this is an automated test.
Output through USB serial port at 115200 bauds.

## Procedure and expected output

1. Reset. Ignore output from the operating system itself.
2. Output must match:

   ```text
   -- GO --
   -- DONE --
   (reset required)
   ```

  It takes a few seconds to run this test and obtain the previous output.
