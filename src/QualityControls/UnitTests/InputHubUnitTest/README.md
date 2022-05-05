# Unit test: Input hub

## Purpose and summary

To test that:

- Clutch bite point is properly stored, then retrieved.
- Button-to-function map is properly stored, then retrieved.
- Clutch works as expected
- ALT function works as expected
- Can enter and exit config menu.
- When menu is enabled, inputs are routed to module "configMenu".

## Harware setup

Nothing required.
Output through USB serial port at 115200 bauds. Connect before reset.

## Procedure and expected output

1. Reset. 

2. Wait (over 30 seconds) for the following line to appear:
   
   ```
   -- END --
   ```

3. Reset again. Ignore output from the operating system itself.

4. Output must match the following:
   
```
-- READY --
-- GO --
- stored preferences -
bite point OK
clutch function OK
ALT function OK

- simulate POV operation (valid input) -
INPUT: 0 Clutch: -127 POV 1
INPUT: 0 Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 5
INPUT: 0 Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 7
INPUT: 0 Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 3
INPUT: 0 Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 8
INPUT: 0 Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 6
INPUT: 0 Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 2
INPUT: 0 Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 4
INPUT: 0 Clutch: -127 POV 0

- simulate POV operation (invalid input) -
INPUT: 0 Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 0

- simulate POV operation while ALT pressed -
INPUT: 64 ALT Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 0
INPUT: 128 ALT Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 0
INPUT: 256 ALT Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 0
INPUT: 512 ALT Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 0

- simulate ALT operation -
INPUT: 0 ALT Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 0
INPUT: 8 Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 0
INPUT: 32 ALT Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 0

- simulate clutch operation (clutch function) -
Bite point: -3
INPUT: 0 Clutch: -3 POV 0
INPUT: 0 Clutch: -127 POV 0
Bite point: -3
INPUT: 0 Clutch: -3 POV 0
INPUT: 0 Clutch: -127 POV 0
INPUT: 0 Clutch: 127 POV 0
INPUT: 0 Clutch: -127 POV 0

- simulate clutch operation (ALT function) -
INPUT: 0 ALT Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 0
INPUT: 0 ALT Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 0
INPUT: 0 ALT Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 0

- simulate clutch operation (BUTTON function) -
INPUT: 2 Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 0
INPUT: 4 Clutch: -127 POV 0
INPUT: 0 Clutch: -127 POV 0

- simulate config menu -
INPUT: 16 Clutch: -127 POV 0
**menu button**
MENU: 8;8
MENU: 0;8
MENU: 16;16
**menu button**

- save preferences -
-- END --
```
