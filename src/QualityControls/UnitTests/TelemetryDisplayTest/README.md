# Unit test: Telemetry display

## Purpose and summary

To test that the notification interface is able to display telemetry data.

## Hardware setup

Nothing required. Output through USB serial port at 115200 bauds. Connect before reset.

## Procedure and expected output

This test runs in a never-ending loop.

1. Reset. Ignore output from the operating system itself.
2. Follow on-screen instructions to select a data rate.
   You must run this test on every option.
3. On each text line, look for `Elapsed: n`,
   where "n" must be close to 1000.
4. Look for `Frame= n` on two consecutive text lines.
   - *Low data rate*:
      "n" should be equal in two consecutive lines,
     then increase by one in the third line.
   - *Medium data rate*:
     "n" should run in strict increasing order with no gaps.
   - *High data rate*:
     "n" should run in increasing order with a gap around 4 units.
