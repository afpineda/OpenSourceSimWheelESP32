# Unit test: Input hub

**NOTE**:: This is also an integration test for modules `inputHub` and `clutchState`.

## Purpose and summary

To test that:

- Function modes are selected in response to the proper input events.
- ALT buttons work as expected.
- Bite point calibration works as expected.
- D-PAD input is properly reported.

## Harware setup

Nothing required. This is an automated test.
Output through USB serial port at 115200 bauds. Connect before reset.

## Procedure and expected output

1. Reset. Ignore output from the operating system itself.
2. Output must match:

   ```text
   -- READY --
   -- GO --
   - simulate POV operation (valid input) -
   - simulate POV operation (invalid input) -
   - simulate POV operation while ALT pushed -
   - simulate cycle ALT function -
   - simulate cycle clutch function -
   - simulate explicit selection of clutch function -
   - simulate non-mapped button combinations -
   - simulate bite point calibration -
   -- END --
   ```
