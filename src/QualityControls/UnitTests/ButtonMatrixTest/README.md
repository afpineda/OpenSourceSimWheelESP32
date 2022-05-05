# Unit test: Input from a button matrix

## Purpouse and summary

To test correct identification of events caused by a button matrix.

## Harware setup

Actual GPIO numbers are defined at [debugUtils.h](./debugUtils.h).
A Button matrix is required, with 3 selector pins and 2 input pins (6 buttons):

- Selector pins at `TEST_BTNMTX_ROW1`, `TEST_BTNMTX_ROW2` and `TEST_BTNMTX_ROW3`.
- Input pins at `TEST_BTNMTX_COL1` and `TEST_BTNMTX_COL1`.
  Output through USB serial port at 115200 bauds.

**Note 1:**
Matrix wiring is prone to human mistake. If something goes weird, check the wiring first.

**Note 2:**
There are multiple valid hardware setups depending on matrix' wiring, so different bitmaps may be assigned to each button in different test boards. That is correct.

For later reference, buttons are named "D1" to "D6", no matter which bitmap is assigned to them.

## Procedure and expected output

### Part 1: mask

1. Reset. Ignore output from the operating system itself.

2. Output must match:
   
   ```
   -- READY --
   MASK:
   1111111111111111111111111111111111111111111111111111111000000111
   -- GO --
   ```

3. Wait for 30 seconds or so. There must be no further output.

### Part 2: single button identification

3. Press and release each button one at a time. A single bit should be set for each pair of output lines. For example (not to be taken literally):
   
   ```
   0000000000000000000000000000000000000000000000000000000010000000
   0000000000000000000000000000000000000000000000000000000000000000
   ```

4. Check that a different bit is set for every button press. For example (not to be taken literally):
   
   ```
   0000000000000000000000000000000000000000000000000000000010000000
   0000000000000000000000000000000000000000000000000000000000000000
   0000000000000000000000000000000000000000000000000000000100000000
   0000000000000000000000000000000000000000000000000000000000000000
   0000000000000000000000000000000000000000000000000000000000100000
   0000000000000000000000000000000000000000000000000000000000000000
   0000000000000000000000000000000000000000000000000000000001000000
   0000000000000000000000000000000000000000000000000000000000000000
   0000000000000000000000000000000000000000000000000000000000001000
   0000000000000000000000000000000000000000000000000000000000000000
   0000000000000000000000000000000000000000000000000000000000010000
   0000000000000000000000000000000000000000000000000000000000000000
   ```

5. Repeat for each button:
   
   - hit and release the button in quick succession, many times.
   - Check that one bit and only one bit is set each time. This is ok.
   - Check that the same bit is set in all button presses. This is ok.

### Part 2: input identification from many buttons

6. Press and hold every button one at a time, from "D1" to "D6". At each step, check that the number of bits set to 1 equals the number of pressed buttons. Last output line must match:
   
   ```
   0000000000000000000000000000000000000000000000000000000111111000
   ```

7. Release every button one at a time, from "D1" to "D6". At each step, check that the number of bits set to 1 equals the number of pressed buttons. Last output line must match:
   
   ```
   0000000000000000000000000000000000000000000000000000000000000000
   ```
