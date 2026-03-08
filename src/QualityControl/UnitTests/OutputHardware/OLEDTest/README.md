# Unit test: monochrome OLED

To test that an OLED strip is properly interfaced via I2C and
is able to display *row-major* vector graphics.

## Hardware setup

Output via USB serial port at 115200 baud.
Serial output is for error messages only
(not strictly required).

A monochrome OLED with 128x64 resolution is required,
but the sketch is tweaked for a 132x64 controller.
Your actual display may require minor changes to the test sketch.

The OLED must be wired to the **primary** I2C bus.
The test sketch will auto-detect the I2C address.
If the OLED is not properly wired or uses a different address,
the message "Device not found. Test cancelled"
will show in the serial monitor.

## Software setup

The [Adafruit GFX](https://docs.arduino.cc/libraries/adafruit-gfx-library/)
library must be installed in Arduino IDE.

## Procedure and expected output

1. Hit `reset`.
2. The OLED must display this pattern:
   - A rectangle in the outer perimeter.
   - A cross from one corner to the opposite.
   - A centered ellipse.

3. Each 3 seconds, all pixels must swap (black to white and vice-versa).
