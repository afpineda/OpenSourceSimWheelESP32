# Integration test: behaviour of some inputs

## Purpose and summary

To test that:

- Button numbers are properly mapped to wheel functions.
- Clutch value is properly set depending on which paddles are pressed or not.
- Bite point is properly set when requested.
- ALT mode is enabled when configured button is pressed.

## Hardware setup

See [debugUtils.h](debugUtils.h) for particular GPIO pin numbers.
Use the same hardware setup as [InputsTest](../InputsTest/README.md).
Output through USB serial port at 115200 bauds.

For later reference:

- "ALT" is the built-in push button into the rotary encoder.
- "CLUTCH1" is the button numbered 1 in the button matrix
- "CLUTCH2" is the button numbered 2 in the button matrix
- "COMMAND" is the button numbered 3 in the button matrix
- "CYCLE_ALT" is the button numbered 4 in the button matrix
- "CYCLE_PADDLES" is the button numbered 5 in the button matrix
- "RCW" means rotary's clockwise rotation
- "RCCW" means rotary's counter-clockwise rotation

**Note:** Before proceeding, identify buttons numbers (1 to 6) at the button matrix. Do not assume they are numbered in any order because they are not, and there is nothing wrong with it.

## Procedure and expected output

1. Reset. Ignore output from the operating system itself.
2. Output must match:

   ```text
   -- READY --
   -- GO --
   ```

3. Press and release "ALT". Output must match:

   ```text
   HID: 0000000000000000000000000000000000000000000000000000000000000000 | 0 (ALT)
   HID: 0000000000000000000000000000000000000000000000000000000000000000 | 0
   ```

4. Press and hold "CLUTCH1", then press and hold "CLUTCH2", then release "CLUTCH1", then release "CLUTCH2". Output must match:

   ```text
   HID: 0000000000000000000000000000000000000000000000000000000000000000 | 127
   HID: 0000000000000000000000000000000000000000000000000000000000000000 | 255
   HID: 0000000000000000000000000000000000000000000000000000000000000000 | 127
   HID: 0000000000000000000000000000000000000000000000000000000000000000 | 0
   ```

5. Press and hold "CLUTCH1", then press and hold "CLUTCH2", then release "CLUTCH2", then release "CLUTCH1". Output must match the previous four lines.

6. Press and hold "CLUTCH2", then press and hold "CLUTCH1", then release "CLUTCH2", then release "CLUTCH1". Output must match the previous four lines.

**UNFINISHED**

7. Press and hold "MENU1" and "MENU2" for three seconds or more, then release both. The **last two lines** of output must match:
   
   ```
   HID RESET
   MENU TOGGLE: 1
   ```

8. Press and release each button in the following order: "ALT", "CLUTCH1, "CLUTCH2", "MENU1" and "MENU2". New output must follow this pattern:
   
   1. Ten lines of text
   
   2. Each line starting width `MENU:`
   
   3. Five of those lines shows a single "1"
   
   4. A line width a single "1" is always followed by this line:
      
      ```
      MENU: 0000000000000000000000000000000000000000000000000000000000000000
      ```
   
   5. Every "1" character takes a different position than the "1" character at other lines

9. Press and hold "MENU1" and "MENU2" for three seconds or more, then release both. The last two lines of output must match:
   
   ```
   HID RESET
   MENU TOGGLE: 0
   ```

10. Press and hold "CLUTCH1", then rotate RCW two detents, then release "CLUTCH1". Output must match:
    
    ```
    HID: 0000000000000000000000000000000000000000000000000000000000000000 | 0
    HID: 0000000000000000000000000000000000000000000000000000000000000000 | 3
    HID: 0000000000000000000000000000000000000000000000000000000000000000 | 3
    HID: 0000000000000000000000000000000000000000000000000000000000000000 | 6
    HID: 0000000000000000000000000000000000000000000000000000000000000000 | 6
    HID: 0000000000000000000000000000000000000000000000000000000000000000 | -127
    ```

11. Press and hold "CLUTCH2", then rotate RCCW three detents, then release "CLUTCH2". Output must match:
    
    ```
    HID: 0000000000000000000000000000000000000000000000000000000000000000 | 6
    HID: 0000000000000000000000000000000000000000000000000000000000000000 | 3
    HID: 0000000000000000000000000000000000000000000000000000000000000000 | 3
    HID: 0000000000000000000000000000000000000000000000000000000000000000 | 0
    HID: 0000000000000000000000000000000000000000000000000000000000000000 | 0
    HID: 0000000000000000000000000000000000000000000000000000000000000000 | -3
    HID: 0000000000000000000000000000000000000000000000000000000000000000 | -3
    HID: 0000000000000000000000000000000000000000000000000000000000000000 | -127
    ```

12. Press and hold "ALT", then press and release "CLUTCH1", then release "ALT". Output must match:
    
    ```
    HID: 0000000000000000000000000000000000000000000000000000000000000000 | -127 (ALT)
    HID: 0000000000000000000000000000000000000000000000000000000000000000 | -3 (ALT)
    HID: 0000000000000000000000000000000000000000000000000000000000000000 | -127 (ALT)
    HID: 0000000000000000000000000000000000000000000000000000000000000000 | -127
    ```

13. Press and hold "MENU1" for three seconds, then release. Output must match:
    
    ```
    HID: 0000000000000000000000000000000000000000000000000000000000000100 | -127
    HID: 0000000000000000000000000000000000000000000000000000000000000000 | -127
    ```

14. Press and hold "MENU2" for three seconds, then release. Output must match:
    
    ```
    HID: 0000000000000000000000000000000000000000000000000000000000001000 | -127
    HID: 0000000000000000000000000000000000000000000000000000000000000000 | -127
    ```

15. Press and hold "MENU1" and "MENU2" for **less than two seconds**, then release both. Any number of lines may output, but all of them must start with `HID:`.