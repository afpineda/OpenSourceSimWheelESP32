# Unit test: Rotary Encoder input

## Purpose and summary

- To test correct identification of rotary encoder events:
  clockwise and counter-clockwise rotation.
  Two consecutive events must happen for each detent
  with a mostly unnoticeable delay between them: STATE = 1, then STATE = 0.
  This is equivalent to a short button press and release.
- To test pulse width multipliers.

## Hardware setup

Use this [test circuit](../../../Protoboards/MainTestBoard.diy):

![Test circuit image](../../../Protoboards/MainTestBoard.png)

Two rotary encoders are tested:

- KY-040 type (with external pull up resistors).
  `CLK` pin attached to `TEST_ROTARY_CLK`.
  `DT` pin attached to `TEST_ROTARY_DT`.
  `V+` and `GND` pins as usual.

- ALPS RKJX series. `Encoder A` pin attached to `TEST_ROTARY_ALPS_A`,
  `Encoder B` pin attached to `TEST_ROTARY_ALPS_B`.
  `Encoder COM` pin attached to `GND`.

If you are not using both rotary encoders,
ensure the missing pins are pulled up (internally or externally).
Otherwise, ghost inputs will appear.

Note that this procedure works the same for both rotary encoders.
Test one first, then the other.

Output through USB serial port at 115200 bauds.

## Procedure and expected output

1. Reset. Ignore output from the operating system itself.
2. Output must show:

   ```text
   -- READY --
   -- GO --
   ```

   You must ignore the `... GPIO isr service already installed` message.
   This is **expected** behavior.

3. Rotate the encoder clockwise (once). Output should be:

   - For the KY-040:

     ```text
     STATE : 0000000000000000000000000000000000000000000000000000000000100000
     STATE : 0000000000000000000000000000000000000000000000000000000000000000
     ```

   - For the funky switch:

     ```text
     STATE : 0000000000000000000000000000000000000000000000000000000000000001
     STATE : 0000000000000000000000000000000000000000000000000000000000000000
     ```

4. Check the correct position of every bit.
5. Rotate the encoder counter-clockwise (once). Output should be:

   - For the KY-040:

     ```text
     STATE : 0000000000000000000000000000000000000000000000000000000001000000
     STATE : 0000000000000000000000000000000000000000000000000000000000000000
     ```

   - For the funky switch:

     ```text
     STATE : 0000000000000000000000000000000000000000000000000000000000000010
     STATE : 0000000000000000000000000000000000000000000000000000000000000000
     ```

6. Check the correct position of every bit.
   Note that output from (5) is not the same as output from (3).
   There is a bit displacement to the left.
7. Clear the serial monitor screen (it makes things easier).
8. Rotate the encoder clockwise many times both fast and slow.
   Check there is output on every single detent.
   Check that all the line pairs are equal.
9. Clear the serial monitor screen (it makes things easier).
10. Rotate the encoder counter-clockwise many times both fast and slow.
    Check there is output on every single detent.
    Check that all the line pairs are equal.
