# Integration test: translation of hardware input events into a HID report

## Purpose and summary

To test that:

- In button mode, clutch paddles are mapped to button numbers when pushed.
- In alternate mode, clutch paddles activate the alternate when pushed.
- In axis mode, clutch paddles are mapped to independent analog axes.
- In clutch mode or launch control mode, clutch paddles are combined into a single axis.
- In clutch mode or launch control mode, bite point calibration works properly.
- In alternate mode, "ALT" buttons engage the alternate mode when pushed.
- In regular button mode, "ALT" buttons are mapped to their button numbers when pushed.

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
