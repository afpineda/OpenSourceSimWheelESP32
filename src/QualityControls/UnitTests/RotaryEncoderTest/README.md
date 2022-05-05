# Unit test: Rotary Encoder input

## Purpouse and summary

To test correct identification of rotary encoder events: clockwise and counter-clockwise rotation. Two consecutive events must happen for each detent with a mostly unnoticeable delay between them: STATE = 1, then STATE = 0. This is equivalent to a short button press and release.

## Harware setup

Actual GPIO numbers are defined at [debugUtils.h](./debugUtils.h).

Relative rotary encoder:

- KY-040 type (with external pull up resistors).
- `CLK` pin attached to `TEST_ROTARY_CLK`
- `DT` pin attached to `TEST_ROTARY_DT`

Output through USB serial port at 115200 bauds.

## Procedure and expected output

1. Reset. Ignore output from the operating system itself. 
2. Output must match:
   
   ```
   -- READY --
   -- GO --
   ```
3. Rotate encoder clockwise (once). Output should be:
   
   ```
   MASK  : 1111111111111111111111111111111111111111111111111111111110011111
   STATE : 0000000000000000000000000000000000000000000000000000000000100000
   CHANGE: 0000000000000000000000000000000000000000000000000000000000100000
   MASK  : 1111111111111111111111111111111111111111111111111111111110011111
   STATE : 0000000000000000000000000000000000000000000000000000000000000000
   CHANGE: 0000000000000000000000000000000000000000000000000000000000100000
   ```
4. Check the correct position of every bit.
5. Rotate encoder counter-clockwise (once). Output should be:
   
   ```
   MASK  : 1111111111111111111111111111111111111111111111111111111110011111
   STATE : 0000000000000000000000000000000000000000000000000000000001000000
   CHANGE: 0000000000000000000000000000000000000000000000000000000001000000
   MASK  : 1111111111111111111111111111111111111111111111111111111110011111
   STATE : 0000000000000000000000000000000000000000000000000000000000000000
   CHANGE: 0000000000000000000000000000000000000000000000000000000001000000
   ```
6. Check the correct position of every bit. Note that output from (4) is not the same as output from (1). There is a bit displacement to the left.
7. Rotate encoder randomly many times both fast and slow. Check there is output on every single detent.
8. Rotate encoder randomly many times at slow pace. Check that output is six and only six lines long with each detent.