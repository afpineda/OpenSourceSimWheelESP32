# Unit test: Input from analog potentiometer

## Purpouse and summary

To test correct reading of analog axis through a potentiometer. This test involves both ADC channels.

## Harware setup

Actual GPIO numbers are defined at [debugUtils.h](./debugUtils.h) as `TEST_ANALOG_PIN1` and `TEST_ANALOG_PIN2`.
A potentiometer is required. Wiring:

- `Pot(+)` to `3V3`.
- `Pot(-)` to `GND`.
- `Pot output` to `TEST_ANALOG_PIN1` and `TEST_ANALOG_PIN2` (both).

## Procedure and expected output

1. Reset. Ignore output from the operating system itself.
2. Output must show:

   ```text
   -- READY --
   -- GO --
   ```

3. Ignore other output.
4. Wait for 30 seconds or so. There must be no further output.
5. Move the potentiometer all the way to `pot(-)`. The last output line must show "(autocalibrated)".
6. Move the potentiometer all the way to `pot(+)`. The last output line must show "(autocalibrated)".
7. Move the potentiometer slowly towards `pot(-)`. The value `xxx` shown at `Axis: xxx` should get lower and lower (down to -127). The word "autocalibrated" must not show up.
8. Move the potentiometer slowly towards `pot(+)`. The value `xxx` shown at `Axis: xxx` should get higher and higher (up to 127). The word "autocalibrated" must not show up.
