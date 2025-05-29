# Unit test: Input from a rotary coded switch

## Purpose and summary

To test correct identification of events caused by a rotary coded switch.

## Hardware setup

There is no need for a physical rotary coded switch.
We are simulating input by wiring input pins to `GND`
as a rotary switch would do.

We are reusing (as input pins) the selector pins
for the testing of analog multiplexers:

- `TEST_AMTXER_SEL1` for the least significant bit.
- `TEST_AMTXER_SEL2`
- `TEST_AMTXER_SEL3` for the most significant bit.

Thus, we simulate a 8-position rotary coded switch.

## Procedure and expected output

Before start, **keep the three wires floating**.

1. Reset. Ignore output from the operating system itself.
2. Output must match:

   ```text
   -- READY --
   MASK:
   1111111111111111111111111111111111111111111111000000001111111111
   -- GO --
   0000000000000000000000000000000000000000000000000000010000000000
   ```

3. Wire all three wires to `GND`. Ignore output.
4. Then, unwire `TEST_AMTXER_SEL3`.
5. Then, unwire `TEST_AMTXER_SEL2`.
6. The last three lines must match up:

   ```text
   0000000000000000000000000000000000000000000000100000000000000000
   0000000000000000000000000000000000000000000000000010000000000000
   0000000000000000000000000000000000000000000000000000100000000000
   ```
