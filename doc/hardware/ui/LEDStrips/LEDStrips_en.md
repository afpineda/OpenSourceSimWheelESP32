# Using RGB LED Strips for raw telemetry display

> [!NOTE] The "pixel control" approach is a better choice to telemetry display using LED strips.

This project supports RGB LED strips to display raw telemetry data.
You can have multiple "gauges" in the same strip,
which are called **segments** here.
For example, let's say you have an LED strip consisting of 10 pixels.
You could have three segments in this strip:

- A "Rev Lights" segment: pixels 1 to 8.
- A flags light segment: pixel 9.
- Another flags light segment: pixel 10.

The full LED strip also indicates some events:
power on, BLE discovery, host connection and low battery.
"Rev lights" segments show changes in bite point calibration.

The recommended display rate is 50 frames per second.

## Firmware customization

The involved class is `LEDStripTelemetry`. Constructor parameters are:

- 1st: GPIO pin number (or alias) for `Dout`.
- 2nd: The total number of pixels available in the LED strip.
- 3rd: logic level shifting.
  - `false` if your LED strip works in 3.3V logic.
  - `true` if your LED strip works in 5V logic
    (so the "level shifter" is in place).
- 4th (optional): pixel driver. Choose a constant from the
  [pixel_driver_t](../../../../src/include/SimWheelTypes.h) enumeration.
- 5th (optional): Pixel data format (byte order):
  - Pass `AUTO` (default) to use the expected pixel format for the given pixel driver.
  - If your LED strip uses another pixel format, use another constant from the
    [pixel_format_t](../../../../src/include/SimWheelTypes.h) enumeration
    describing the red, green and blue byte order.

For example:

```c++
auto ui = new LEDStripTelemetry(GPIO_NUM_13, 10, true, WS2812, BGR);
```

Create each *segment* as described below,
passing the `LEDStripTelemetry` instance as the first parameter.
Other parameters depend on the *segment*.

For example:

```c++
new RevLightsLEDSegment(ui,0,8);
new RaceFlagsLEDSegment(ui,8);
new RaceFlagsLEDSegment(ui,9);
```

Finally, pass the `LEDStripTelemetry` instance to `notify::begin()`.

**Take into account**:

- All instances must be created with `new`.
  Don't delete.
- Pixels are indexed starting from zero.
- Don't overlap *segments*.
  Use different pixels for each *segment*.
- Non-existent pixels are ignored without warning.
- You can set the **global LED brightness** by calling `brightness()` with a parameter.
  More on this below.

Have a look at the code for the specific integration tests for an example:

- [Rev lights](../../../../src/QualityControls/UITests/RevLightsSegmentTest/RevLightsSegmentTest.ino)
- [Other single-pixel segments](../../../../src/QualityControls/UITests/SinglePixelSegmentsTest/SinglePixelSegmentsTest.ino)

### "Rev lights"

The `RevLightsLEDSegment` class shows an RPM bar.
The color depends on the maximum torque and power.
It will flash at the rev limiter.

Constructor parameters are:

- 1st: `LEDStripTelemetry` instance.
- 2nd: index of the first pixel.
- 3rd: number of consecutive pixels in this segment.
- 4th (optional): main bar color (RGB).
  Default is green.
- 5th (optional): bar color when maximum torque has been reached (RGB).
  Default is yellow.
- 6th (optional): bar color when maximum power has been reached (RGB).
  Default is red.
- 7th (optional): bar color to notify current bite point (RGB).
  Pass zero to disable. Default is white.
- 8th (optional): display mode (left to right, right to left, center to edges, or edges to center).
  A constant in the `revLightsMode_t` enumeration.
  Default is left to right.

### Race flags

The `RaceFlagsLEDSegment` class shows race flags in a single pixel.
By default, black and checkered flags are not shown (there is no color for them).
You may adjust a blink rate or disable blinking.
Flag display priority is (from highest to lowest):

- Blue
- Yellow
- White
- Green
- Orange
- Black
- Checkered

Constructor parameters are:

- 1st: `LEDStripTelemetry` instance.
- 2nd: Pixel index.
- 3rd (optional): Blink rate in milliseconds.
  Pass zero to disable blinking. Default is 500 ms.

Additionally, you can set the exact flag color by setting the
`RaceFlagsLEDSegment::color_*` static variables
(see [SimwheelUI.h](../../../../src/include/SimWheelUI.h)).
Use this trick to adjust the brightness independent of the global brightness.

### Shift light

The `ShiftLightLEDSegment` class displays shift information in a single pixel.
The pixel lights green when maximum torque is reached,
then red when maximum power is reached,
then flashes when the rev limiter is reached.
Colors are customizable.

Constructor parameters are:

- 1st: `LEDStripTelemetry` instance.
- 2nd: Pixel index.
- 3rd (optional): Color for maximum torque (RGB).
- 4th (optional): Color for maximum power (RGB).

### Witnesses

The `WitnessLEDSegment` class displays witness information from the ECU:
TC, ABS, DRS, pit limiter or low fuel.
You can have up to three witnesses in a single pixel.
The first takes precedence over the second and so on.
Colors are customizable.

Constructor parameters are:

- 1st: `LEDStripTelemetry` instance.
- 2nd: Pixel index.
- 3rd: First-priority witness.
  Pass one of the constants in the
  [witness_t](../../../../src/include/SimWheelUI.h) enumeration.
- 4th: Color for the first-priority witness.
- 5th (optional): Second-priority witness.
  Pass `NONE` to ignore.
- 6th (optional): Color for the second-priority witness.
- 7th (optional): Third-priority witness.
  Pass `NONE` to ignore.
- 8th (optional): Color for the third-priority witness.

Typically, you will ignore the 5th and subsequent parameters to create a single witness light.

## Brightness

RGB LED strips are so bright that they are harmful to the eyes.
You should use low brightness pixel colors for a comfortable user experience.
For example, both `0xFFFFFF` and `0x7F7F7F` are the RGB representation of the color white.
However, the former is at full brightness and the latter is at half brightness.

The API will reduce any pixel brightness to the proportional value set by `brightness()`.
For example, `0xFFFFFFFF` becomes `0x202020` when using a global brightness of 32 (decimal).
For fine tuning, set the global brightness to `0xFF` (full brightness)
and then provide the per pixel brightness via color data.

The default global brightness setting is 15.
Unfortunately, a low global brightness may prevent some colors from appearing.
The entire LED strip will turn off if you set the global brightness to zero.
