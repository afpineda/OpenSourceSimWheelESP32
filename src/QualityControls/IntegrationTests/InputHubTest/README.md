# Integration test: behavior of some inputs

## Purpose and summary

To test that:

- In button mode, clutch paddles are mapped to button numbers when pushed.
- In "ALT" mode, clutch paddles activate the "ALT" function when pushed.
- In axis mode, clutch paddles are mapped to independent analog axes.
- In clutch mode or launch control mode, clutch paddles are combined into a single axis.
- In clutch mode or launch control mode, bite point calibration works properly.
- Configured button combinations works properly when selecting wheel functions.
- In "ALT" mode, "ALT" buttons engage the "ALT" function when pushed.
- In button mode, "ALT" buttons are mapped to their button numbers when pushed.

## Hardware setup

Actual GPIO numbers are defined at [debugUtils.h](./debugUtils.h).
Use this [test circuit](../../Protoboards/TestBoard1.diy):

![Test circuit image](../../Protoboards/TestBoard1.png)

We are using the KY-040 rotary encoder, both clutch potentiometers and the button matrix.

For later reference:

- "ALT" is the built-in push button into the rotary encoder, button number 10.
- "LCLUTCH" and "RCLUTCH" are the analog potentiometers (numbered 0 and 1)
  for the left and right clutch paddles, respectively.
- "COMMAND" is the button number 2 in the button matrix (see picture).
- "CYCLE_ALT" is the button number 3 in the button matrix.
- "CYCLE_PADDLES" is the button number 4 in the button matrix.
- "RCW" means rotary's clockwise rotation (number 12).
- "RCCW" means rotary's counter-clockwise rotation (number 13).

Output has this format:

  ```text
  ...<high 16 bits> ...<low 16 bits> | <L> | <R> | <C> || <BP>
  ```

Where:

- `<low 16 bits>` is the input bitmap for buttons 0 to 15, which are reported when ALT is not engaged.
- `<high 16 bits>` is the input bitmap for buttons 64 to 80, which are the same as buttons 0 to 15,
  but reported when ALT is engaged.
- `<L>` is the axis value from the left potentiometer.
- `<R>` is the axis value from the right potentiometer.
- `<C>` is the combined axis value from both potentiometers.
- `<BP>` is the current bite point.

Note that `<L>` and `<R>` are interchangeable.
Output through USB serial port at 115200 bauds.

## Procedure and expected output

1. Reset. Ignore output from the operating system itself.
2. Output must match:

   ```text
   -- READY --
   -- GO --
   ```

3. Move both potentiometers from one end to the other (calibration). Ignore output.
4. Move "LCLUTCH" to full extent, then "RCLUTCH" to full extent,
   then "LCLUTCH" to idle, then "RCLUTCH" to idle.
5. Not to be taken literally, output must show the following lines in any order:

   ```text
   ...0000000000000000 ...0000000000000010 | 000 | 000 | 000 || 127
   ...0000000000000000 ...0000000000000011 | 000 | 000 | 000 || 127
   ...0000000000000000 ...0000000000000001 | 000 | 000 | 000 || 127
   ...0000000000000000 ...0000000000000000 | 000 | 000 | 000 || 127
   ```

6. Push and release "ALT".
   Nothing should happen.

7. Push and hold "COMMAND", then "RCW", then release "COMMAND".
   Output must show:

   ```text
   ...0000000000000000 ...0000000000000100 | 000 | 000 | 000 || 127
   ...0000000000000000 ...0001000000000100 | 000 | 000 | 000 || 127
   ...0000000000000000 ...0000000000000100 | 000 | 000 | 000 || 127
   ...0000000000000000 ...0000000000000000 | 000 | 000 | 000 || 127
   ```

8. Push "COMMAND" and "CYCLE_ALT" at the same time, then release.
   Output must show `ALT mode: 0`, along other lines.

9. Push and release "ALT". Output must show:

   ```text
   ...0000000000000000 ...0000010000000000 | 000 | 000 | 000 || 127
   ...0000000000000000 ...0000000000000000 | 000 | 000 | 000 || 127
   ```

10. Push "COMMAND" and "CYCLE_ALT" at the same time, then release.
    Output must show `ALT mode: 1`, along other lines.

11. Push "COMMAND" and "CYCLE_PADDLES" at the same time, then release.
    Output must show `Clutch mode: 4`
    (meaning "launch control - master is left"),
    along other lines.

12. Move `RCLUTCH` from end to end and back to idle.
    Look at the output lines.
    The field `<C>` must hold `000` or `127`,
    but no other value.

13. Move `LCLUTCH` slowly from end to end and back to idle.
    Look at the output lines.
    The field `<C>` must show `000` and `254`,
    along many intermediate values.

14. Push "COMMAND" and "CYCLE_PADDLES" at the same time, then release.
    Output must show `Clutch mode: 5`
    (meaning "launch control - master is right"),
    along other lines.

15. Move `LCLUTCH` from end to end and back to idle.
    Look at the output lines.
    The field `<C>` must hold `000` or `127`,
    but no other value.

16. Move `RCLUTCH` slowly from end to end and back to idle.
    Look at the output lines.
    The field `<C>` must show `000` and `254`,
    along many intermediate values.

17. Move `LCLUTCH` to full engagaged and keep `RCLUTCH` idle.
18. Rotate "RCCW" just one time.
    The field `<BP>` must show `124`.
19. Rotate "CCW" just one time.
    The field `<BP>` must show `127` again.

20. Push "COMMAND" and "CYCLE_PADDLES" at the same time, then release.
    Output must show `Clutch mode: 0` (meaning "clutch mode"),
    along other lines.

21. Ensure both potentiometers are idle.
22. Move "LCLUTCH" slowly to full extent.
    Output must show an increasing number at `<C>` up to 127.
    `<L>` and `<R>` must remain in zero.
23. Move "RCLUTCH" slowly to full extent. Output must show an increasing number at `<C>`,
    greater than 127, and up to 254.
24. Move "LCLUTCH" slowly to idle.
    Output must show a decreasing number at `<C>` down to 127.
25. Keep "RCLUTCH" at full extent.
26. Rotate "RCCW" many times.
    Output must show a decreasing number at `<BP>` down to 1.
    All binary digits must remain in 0.
27. Rotate "CCW" many times. Output must show an increasing number at `<BP>` up to 254.
    All binary digits must remain in 0.
28. Move "RCLUTCH" to idle.
29. Push "COMMAND" and "CYCLE_PADDLES" at the same time, then release.
    Output must show `Clutch mode: 1` (meaning "axis mode").
30. Output must show `<C>` set to 0.
31. Move "LCLUTCH" at random.
    Output must show a changing value at `<L>`, and `<C>` set to 0.
32. Move "RCLUTCH" at random.
    Output must show a changing value at `<R>`, and `<C>` set to 0.
33. Move both "LCLUTCH" and "RCLUTCH" to idle.
34. Push "COMMAND" and "CYCLE_PADDLES" at the same time, then release.
    Output must show `Clutch mode: 2` (meaning "Alternate mode").
35. Move "LCLUTCH" to full extent, then back to idle. Nothing should happen.

36. Push "COMMAND" and "CYCLE_PADDLES" at the same time, then release.
    Output must show `Clutch mode: 3` (meaning "button mode").
