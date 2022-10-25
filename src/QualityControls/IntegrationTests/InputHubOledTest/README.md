# Integration test: OLED feedback in response to user input

## Purpose and summary

To test that menu and clutch operation shows the proper feedback at the user interface.

## Hardware setup

Use the same hardware setup as both [InputHubTest](../InputHubTest/README.md) and [uiTest](../uiTest/README.md): OLED display, button matrix and rotary encoder.

For reference:

- "ALT" is the integrated push button into the rotary encoder.
- "CLUTCH1" is the button numbered 1 in the button matrix
- "CLUTCH2" is the button numbered 2 in the button matrix
- "MENU" is the button numbered 3 in the button matrix
- "RCW" means rotary's clockwise rotation
- "RCCW" means rotary's counter-clockwise rotation

**Note:** Before proceeding, identify which buttons are numbered 1, 2, and 3 at the button matrix. Do not assume they are numbered in any order because they are not, and there is nothing wrong with it.

## Procedure and expected output

1. Reset.
2. The splash screen should appear at the OLED.
3. Press and release "CLUTCH1". OLED must display the "Bite Point-50%" screen.
4. Wait for 5 seconds or so.
5. OLED must be cleared.
6. Press and hold the "MENU" button for 5 seconds, then release. OLED must display the "Menu-Exit" screen.
7. Rotate RCW 8 times. On each detent the following screens must appear in sequence:
   1. "Menu-language"
   2. "Menu-clutch paddles"
   3. "Menu-ALT buttons"
   4. "Menu-Load preset"
   5. "Menu-Save preset"
   6. "Menu-Power off"
   7. "Menu-Frameserver"
   8. "Menu-Exit"
8. Rotate RCCW 8 times. On each detent the following screens must appear in sequence:
   1. "Menu-Frameserver"
   2. "Menu-Power off"
   3. "Menu-Save preset"
   4. "Menu-Load preset"
   5. "Menu-ALT buttons"
   6. "Menu-clutch paddles"
   7. "Menu-language"
   8. "Menu-Exit"
9. Go to the "Menu-Load preset" screen. Press and release "ALT". OLED must display the "Load preset-Slot 01" screen.
10. Rotate RCW 3 times, then rotate RCCW 4 times. On each detent, the following screens must appear in sequence:
    1. "Load preset-Slot 02"
    2. "Load preset-Slot 03"
    3. "Load preset-Slot 04"
    4. "Load preset-Slot 03"
    5. "Load preset-Slot 02"
    6. "Load preset-Slot 01"
    7. "Load preset-Slot 01" (again)
11. Press and release the "MENU" button. OLED must display the "Menu-Load preset" screen.
12. Press and release the "MENU" button again. OLED must display the "Menu-exit" screen.
13. Go to the "Menu-save preset" screen. Press and release "ALT".  OLED must display the "Save preset-Slot 01" screen.
14. Press and release "ALT". The same screen with a flashing edge must show for a second. Then, OLED must display the "Menu-Exit" screen.
15. Press and release "ALT". OLED must be cleared.
16. Press and hold "CLUTCH2", then rotate RCW 4 times.
17. Oled must the display "Bite point-54%" screen.
18. Wait for 5 seconds or so. OLED must be cleared.
19. Wait for 30 seconds or so. OLED must display the "Saved!" screen.
20. Wait for 5 seconds or so. OLED must be cleared.
