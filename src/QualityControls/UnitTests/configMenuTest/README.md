# Unit test: config menu

## Purpose and summary

To test that:

- Menu is hidden or shown as required
- Inputs work as intended in order to navigate through menus, select and cancel.

## Hardware setup

Nothing required. Output through USB serial port at 115200 bauds.

## Procedure and expected output

1. Reset. Ignore output from the operating system itself.
2. Output must match:
   
   ```
   -- READY --
   -- GO --
   **TEST #1
   **TEST #2
   **TEST #3
   **TEST #4
   **TEST #5
   **TEST #6
   **TEST #7
   **TEST #8
   **TEST #9
   **TEST #10
   **TEST #11
   **TEST #12
   **TEST #13
   -- END --
   ```
