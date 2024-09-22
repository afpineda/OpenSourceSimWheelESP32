# Unit test: Telemetry display

## Purpose and summary

To test that the notification interface is able to display telemetry data.

## Hardware setup

Nothing required. Output through USB serial port at 115200 bauds. Connect before reset.

## Procedure and expected output

This test runs in a never-ending loop.

1. Reset. Ignore output from the operating system itself.
2. The following text must appear:

   ```text
   -- READY --
   (DEVICE READY)
   ```

3. From now on, ignore all `(FRAME n MS)` text lines.
4. `(TELEMETRY ID n)` must show up, where "n" an integer number,
   different to previous ones.
5. `(TELEMETRY ID m)` must follow, being $m = n+1$.
6. `(TELEMETRY OFF)` must follow.
7. Back to 4.
