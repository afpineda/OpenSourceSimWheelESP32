# Unit test: user interface subsystem

## Purpose and summary

To test that the user interface subsystem is able to:

- Run two user interfaces in parallel.
- Receive notifications.
- Receive telemetry data.
- Run a frameserver.
- Shutdown the output hardware.

## Hardware setup

Output through the USB serial port at 115200 bauds.

## Procedure and expected output

This is an automated test.

1. Reset. Ignore output from the operating system itself.
2. Output must match:

   ```text
   --READY--
   --GO--
   --DONE--
   ```

3. Just check there are no `ERROR` messages.
