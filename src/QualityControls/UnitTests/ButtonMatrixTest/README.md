# Unit test: Input from a button matrix

## Purpouse and summary

To test correct identification of events caused by a button matrix.

## Harware setup

Actual GPIO numbers are defined at [debugUtils.h](./debugUtils.h).
Use this [test circuit](../../Protoboards/ESP32-WROOM-DevKitC-1.diy):

![Test circuit image](../../Protoboards/ProtoBoard-ESP32-Dekvit-C-1.png)

- We are using the button matrix only: `TEST_BTNMTX_ROW1`, `TEST_BTNMTX_ROW2`, `TEST_BTNMTX_COL1`, `TEST_BTNMTX_COL2` and `TEST_BTNMTX_COL3`.
- Matrix wiring is prone to human mistake. If something goes weird, check the wiring first.
- For later reference, buttons are named "D1" to "D6", no matter which bitmap is assigned to them.

## Procedure and expected output

### Part 1: mask

1. Reset. Ignore output from the operating system itself.

2. Output must match:

   ```text
   -- READY --
   MASK:
   1111111111111111111111111111111111111111111111111111111100000011
   -- GO --
   ```

3. Wait for 30 seconds or so. There must be no further output.

### Part 2: single button identification

1. Press and release each button one at a time. A single bit should be set for each pair of output lines. For example (not to be taken literally):

   ```text
   0000000000000000000000000000000000000000000000000000000010000000
   0000000000000000000000000000000000000000000000000000000000000000
   ```

2. Check that a different bit is set for every button press. For example (not to be taken literally):

   ```text
   0000000000000000000000000000000000000000000000000000000001000000
   0000000000000000000000000000000000000000000000000000000000000000
   0000000000000000000000000000000000000000000000000000000010000000
   0000000000000000000000000000000000000000000000000000000000000000
   0000000000000000000000000000000000000000000000000000000000010000
   0000000000000000000000000000000000000000000000000000000000000000
   0000000000000000000000000000000000000000000000000000000000100000
   0000000000000000000000000000000000000000000000000000000000000000
   0000000000000000000000000000000000000000000000000000000000000100
   0000000000000000000000000000000000000000000000000000000000000000
   0000000000000000000000000000000000000000000000000000000000001000
   0000000000000000000000000000000000000000000000000000000000000000
   ```

3. Repeat for each button:

   - hit and release the button in quick succession, many times.
   - Check that one bit and only one bit is set each time. This is ok.
   - Check that the same bit is set in all button presses. This is ok.

### Part 2: input identification from many buttons

1. Press and hold every button one at a time, from "D1" to "D6". At each step, check that the number of bits set to 1 equals the number of pressed buttons. Last output line must match:

   ```text
   0000000000000000000000000000000000000000000000000000000011111100
   ```

2. Release every button one at a time, from "D1" to "D6". At each step, check that the number of bits set to 1 equals the number of pressed buttons. Last output line must match:

   ```text
   0000000000000000000000000000000000000000000000000000000000000000
   ```
