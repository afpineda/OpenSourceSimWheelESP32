# RGB LED Strips

This project supports RGB LED strips for telemetry display.
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

## Hardware design

These LED strips are controlled by a single output-capable pin labelled `Dout` in this project.
Some LED strips can work with 3.3V logic, others cannot.
If your LED strip does not work with 3.3V logic,
you will need a small "level shifter" circuit, which is described below.

- **No level shifter (3.3V logic)**

  Simply wire `Dout` to `Din` in the first pixel.

  ![No level shifter](./NoLevelShifter.png)

- **Level shifter (5V logic)**

  `Din` requires a minimum voltage of 3.5 volts.
  The ESP32 operates at 3.3 volts, which is insufficient.
  To overcome this limitation, a "level shifter" circuit is required.
  We are using the most simple level shifter available:
  a pull resistor attached to a GPIO pin in *open drain* mode (`Dout`).

  ![Level shifter](./LevelShifter.png)

  [Test this circuit at falstad.com](https://falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWcMBMcUHYMGZIA4UA2ATmIxAUgoqoQFMBaMMAKACcQG8rvPenCUZHBYAPTmF4YUEgCzkMSWeEEARAPYBXAC4sA7hMEDDIQssgsASnyrGmYGcapVZtIc+gIxEtJyKcMPD9sEGVJEAA1ABk6ABNvJhC8QTAkPGJQ8CDVAEkAOxYAI05sBEd+SSQUbHILYoYUZVkMlGIqZhkLcQbiIIZZPsbbWSUQX1U6ADcAaQBLXXFmIOSxuBB0zN8ogFFVAB0AZwPtNlmAB30TYiMwFJQgiwBzEwc+25VnKASHU2UZQnIYRkCAi33akBSKUgGSBIAA4gA5VSXYz8Xi8FAoio3HFfAxcHi2NDLZwJbDvMDSCTEMKKTI-AAU6jOdDyh1ibAAhrM2UdZgBbTQAG052lm6jyAEoWAc-Ph1kSqa93CATpo6Cxng0lfc-ErZOYvvVcMozHKZKkHiwgA)

  Needed parts:

  - 1K-ohms resistor (x1)

## Firmware customization

The involved class is `LEDStripTelemetry`. Constructor parameters are:

- 1st: GPIO pin number (or alias) for `Dout`.
- 2nd: The total number of pixels available in the LED strip.
- 3rd: logic level shifting.
  - `false` if your LED strip works in 3.3V logic.
  - `true` if your LED strip works in 5V logic
    (so the "level shifter" is in place).
- 4th (optional): Pixel driver. The firmware supports several pixel drivers:
  - Pass `WS2811` for
    [WS2811](https://www.alldatasheet.com/datasheet-pdf/pdf/1132633/WORLDSEMI/WS2811.html)
    drivers.
    *Note*: not tested.
  - Pass `WS2812` (default) for any driver in that [family](http://world-semi.com/ws2812-family/),
    including:
    [WS2812](https://www.alldatasheet.com/datasheet-pdf/pdf/1134521/WORLDSEMI/WS2812.html) and
    [WS2812B](https://www.alldatasheet.com/datasheet-pdf/pdf/1179113/WORLDSEMI/WS2812B.html)
    drivers.
  - Pass `WS2815` for any driver in that [family](http://world-semi.com/ws2815-family/).
    *Note*: not tested.
  - Pass `SK6812` for
    [SK6812](https://cdn-shop.adafruit.com/product-files/1138/SK6812%20LED%20datasheet%20.pdf)
    drivers.
    *Note*: not tested.
  - Pass `UCS1903` for
    [UCS1903](https://www.led-stuebchen.de/download/UCS1903_English.pdf)
    drivers.
    *Note*: not tested.
- 5th (optional): Pixel data format (byte order):
  - Pass `AUTO` (default) to use the expected pixel format for the given pixel driver.
  - If your LED strip uses another pixel format, use another constant from the
    [pixel_format_t](../../../../src/include/LedStrip.h) enumeration,
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
