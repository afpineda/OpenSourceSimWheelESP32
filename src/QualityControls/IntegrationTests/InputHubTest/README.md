# Integration test: behaviour of some inputs

## Purpose and summary

To test that:

- In button mode, clutch paddles are mapped to button numbers when pushed.
- In "ALT" mode, clutch paddles activate the "ALT" function when pushed.
- In axis mode, clutch paddles are mapped to independent analog axes.
- In clutch mode, clutch paddles are combinend into a single axis.
- In clutch mode, bite point calibration works properly.
- Configured button combinations works properly when selecting wheel functions.
- In "ALT" mode, "ALT" buttons activate the "ALT" function when pushed.
- In button" mode, "ALT" buttons are mapped to their button numbers when pushed.

## Hardware setup

Actual GPIO numbers are defined at [debugUtils.h](./debugUtils.h).
Use this [test circuit](../../Protoboards/ESP32-WROOM-DevKitC-1.diy):

![Test circuit image](../../Protoboards/ProtoBoard-ESP32-Dekvit-C-1.png)

We are using the KY-040 rotary encoder, both clutch potentiometers and the button matrix.

For later reference:

- "ALT" is the built-in push button into the rotary encoder, button number 10.
- "CLUTCH1" and "CLUTCH2" are the analog potentiometers (numbered 0 and 1).
- "COMMAND" is the button number 2 in the button matrix.
- "CYCLE_ALT" is the button number 3 in the button matrix.
- "CYCLE_PADDLES" is the button number 4 in the button matrix.
- "RCW" means rotary's clockwise rotation (number 12).
- "RCCW" means rotary's counter-clockwise rotation (number 13).
- Output has this format:

  ```text
  <64 binary digits> | <C> | <L> | <R> || <BP>
  ```

  Where:

  - `<C>` is the combined axis value from both potentiometers.
  - `<L>` is the axis value from the left potentiometer.
  - `<R>` is the axis value from the right potentiometer.
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
4. Move "CLUTCH1" to full extent, then "CLUTCH2" to full extent, then "CLUTCH1" to idle, then "CLUTCH2" to idle.
5. Not to be taken literally, output must show the following lines in any order:

   ```text
   0000000000000000000000000000000000000000000000000000000000000010 | 0 | 0 | 0 || 127
   0000000000000000000000000000000000000000000000000000000000000011 | 0 | 0 | 0 || 127
   0000000000000000000000000000000000000000000000000000000000000001 | 0 | 0 | 0 || 127
   0000000000000000000000000000000000000000000000000000000000000000 | 0 | 0 | 0 || 127
   ```

6. Push and release "ALT". Output must show:

   ```text
   0000000000000000000000000000000000000000000000000000000000000000 | 0 | 0 | 0 || 127 (ALT)
   0000000000000000000000000000000000000000000000000000000000000000 | 0 | 0 | 0 || 127
   ```

7. Push and hold "COMMAND", then "RCW", then release "COMMAND". Output must show:

   ```text
   0000000000000000000000000000000000000000000000000000000000000100 | 0 | 0 | 0 || 127
   0000000000000000000000000000000000000000000000000001000000000100 | 0 | 0 | 0 || 127
   0000000000000000000000000000000000000000000000000000000000000100 | 0 | 0 | 0 || 127
   0000000000000000000000000000000000000000000000000000000000000000 | 0 | 0 | 0 || 127
   ```

8. Push "COMMAND" and "CYCLE_ALT" at the same time, then release. Output must show `ALT mode: 0`.
9. Push and release "ALT". Output must show:

   ```text
   0000000000000000000000000000000000000000000000000000010000000000 | 0 | 0 | 0 || 127
   0000000000000000000000000000000000000000000000000000000000000000 | 0 | 0 | 0 || 127
   ```

10. Push "COMMAND" and "CYCLE_ALT" at the same time, then release. Output must show `ALT mode: 1`.
11. Push "COMMAND" and "CYCLE_PADDLES" at the same time, then release. Output must show `Clutch mode: 0`.
12. Ensure both potentiometers are idle.
13. Move "CLUTCH1" slowly to full extent. Output must show an increasing number at `<C>` up to 127. `<L>` and `<R>` must remain in zero.
14. Move "CLUTCH2" slowly to full extent. Output must show an increasing number at `<C>`, greater than 127, and up to 254.
15. Move "CLUTCH1" slowly to idle. Output must show a decreasing number at `<C>` down to 127.
16. Keep "CLUTCH2" at full extent.
17. Rotate "RCCW" many times. Output must show a decreasing number at `<BP>` down to 1. All binary digits must remain in 0.
18. Rotate "CCW" many times. Output must show an increasing number at `<BP>` up to 253. All binary digits must remain in 0.
19. Move "CLUTCH2" to idle.
20. Push "COMMAND" and "CYCLE_PADDLES" at the same time, then release. Output must show `Clutch mode: 1`.
21. Output must show `<C>` set to 0.
22. Move "CLUTCH1" at random. Output must show a changing value at `<L>` and `<C>` set to 0.
23. Move "CLUTCH2" at random. Output must show a changing value at `<R>` and `<C>` set to 0.
24. Move both "CLUTCH1" and "CLUTCH2" to idle.
25. Push "COMMAND" and "CYCLE_PADDLES" at the same time, then release. Output must show `Clutch mode: 2`.
26. Move "CLUTCH1" to full extent, then back to iddle. Output must show:

    ```text
    0000000000000000000000000000000000000000000000000000000000000000 | 0 | 0 | 0 || 253 (ALT)
    0000000000000000000000000000000000000000000000000000000000000000 | 0 | 0 | 0 || 253
    ```

27. Push "COMMAND" and "CYCLE_PADDLES" at the same time, then release. Output must show `Clutch mode: 3`.
