# Unit test: Rotary Encoder input

## Purpose and summary

To test correct identification of rotary encoder events: clockwise and counter-clockwise rotation. Two consecutive events must happen for each detent with a mostly unnoticeable delay between them: STATE = 1, then STATE = 0. This is equivalent to a short button press and release.

## Hardware setup

Actual GPIO numbers are defined at [debugUtils.h](./debugUtils.h).
Use this [test circuit](../../Protoboards/TestBoard1.diy):

![Test circuit image](../../Protoboards/TestBoard1.png)

Two rotary encoders are tested:

- KY-040 type (with external pull up resistors). `CLK` pin attached to `TEST_ROTARY_CLK`. `DT` pin attached to `TEST_ROTARY_DT`. `V+` and `GND` pins as usual.
- ALPS RKJX series. `Encoder A` pin attached to `TEST_ROTARY_ALPS_A`, `Encoder B` pin attached to `TEST_ROTARY_ALPS_B`. `Encoder COM` pin attached to `GND`.

If you are not using both rotary encoders, ensure the missing pins are attached to `3V3`. Otherwise, ghost inputs will appear.

Note that this procedure works the same for both rotary encoders. Test one first, then the other.

Output through USB serial port at 115200 bauds.

## Procedure and expected output for the KY-040 rotary encoder

1. Reset. Ignore output from the operating system itself.
2. Output must match:

   ```text
   -- READY --
   -- GO --
   ```

3. Rotate encoder clockwise (once). Output should be:

   ```text
   MASK  : 1111111111111111111111111111111111111111111111111111111110011100
   STATE : 0000000000000000000000000000000000000000000000000000000000100000
   CHANGE: 0000000000000000000000000000000000000000000000000000000000100000
   MASK  : 1111111111111111111111111111111111111111111111111111111110011100
   STATE : 0000000000000000000000000000000000000000000000000000000000000000
   CHANGE: 0000000000000000000000000000000000000000000000000000000000100000
   ```

4. Check the correct position of every bit.
5. Rotate encoder counter-clockwise (once). Output should be:

   ```text
   MASK  : 1111111111111111111111111111111111111111111111111111111110011100
   STATE : 0000000000000000000000000000000000000000000000000000000001000000
   CHANGE: 0000000000000000000000000000000000000000000000000000000001000000
   MASK  : 1111111111111111111111111111111111111111111111111111111110011100
   STATE : 0000000000000000000000000000000000000000000000000000000000000000
   CHANGE: 0000000000000000000000000000000000000000000000000000000001000000
   ```

6. Check the correct position of every bit. Note that output from (4) is not the same as output from (1). There is a bit displacement to the left.
7. Rotate encoder randomly many times both fast and slow. Check there is output on every single detent.
8. Rotate encoder randomly many times at slow pace. Check that output is six and only six lines long with each detent.

## Procedure and expected output for the ALPS rotary encoder

1. Reset. Ignore output from the operating system itself.
2. Output must match:

   ```text
   -- READY --
   -- GO --
   ```

3. Rotate encoder clockwise (once). Output should be:

   ```text
   MASK  : 1111111111111111111111111111111111111111111111111111111110011100
   STATE : 0000000000000000000000000000000000000000000000000000000000000001
   CHANGE: 0000000000000000000000000000000000000000000000000000000000000001
   MASK  : 1111111111111111111111111111111111111111111111111111111110011100
   STATE : 0000000000000000000000000000000000000000000000000000000000000000
   CHANGE: 0000000000000000000000000000000000000000000000000000000000000001
   ```

4. Check the correct position of every bit.
5. Rotate encoder counter-clockwise (once). Output should be:

   ```text
   MASK  : 1111111111111111111111111111111111111111111111111111111110011100
   STATE : 0000000000000000000000000000000000000000000000000000000000000010
   CHANGE: 0000000000000000000000000000000000000000000000000000000000000010
   MASK  : 1111111111111111111111111111111111111111111111111111111110011100
   STATE : 0000000000000000000000000000000000000000000000000000000000000000
   CHANGE: 0000000000000000000000000000000000000000000000000000000000000010
   ```

6. Check the correct position of every bit. Note that output from (4) is not the same as output from (1). There is a bit displacement to the left.
7. Rotate encoder randomly many times both fast and slow. Check there is output on every single detent.
8. Rotate encoder randomly many times at slow pace. Check that output is six and only six lines long with each detent.

## Pulse width testing (any rotary encoder)

1. Type `2` in the serial monitor and hit `enter`.
2. Output should be:

    ```text
    Pulse multiplier set to 2
    ```

3. Rotate clockwise or counter-clockwise (it doesn't matter).
   Ignoring `...`, output should be:

   ```text
   MASK  : ...
   STATE : ...
   CHANGE: ...
   Pulse delay
   MASK  : ...
   STATE : ...
   CHANGE: ...
   ```

4. Type `3` in the serial monitor and hit `enter`.
    Output should be:

    ```text
    Pulse multiplier set to 3
    ```

5. Rotate clockwise or counter-clockwise (it doesn't matter).
   Ignoring `...`, output should be:

   ```text
   MASK  : ...
   STATE : ...
   CHANGE: ...
   Pulse delay
   Pulse delay
   MASK  : ...
   STATE : ...
   CHANGE: ...
   ```
