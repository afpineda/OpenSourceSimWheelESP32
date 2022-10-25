# Unit test: Frame server output

## Purpose and summary

To test that:

- the frame server is properly enabled and disabled
- when enabled, the frame server is able to display simulation data properly

## Hardware setup

OLED attached to `SCL` and `SDA` as usual.

## Procedure and expected output

1. Reset. Wait for the "Welcome" screen to disappear.
2. The following sequence must be displayed in a loop:
   1. Gear "N". Shown for 2 seconds.
   2. Speed going up from 0 to 500, gear "1":
      1. Gear going up every 100.
      2. Gear showing braces "[]" near each multiple of 100.
      3. ABS going up from 0 to 90 and back to 0 in a loop.
      4. TC going up from 0 to 20 and back to 0 in a loop.
      5. MAP going up from 0 to 10 and back to 0 in a loop.
   3. Screen cleared for 5 seconds.
