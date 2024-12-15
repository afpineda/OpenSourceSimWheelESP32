# Firmware customization in the *pixel control* approach

Place up to three calls to `pixels::configure()` with the following parameters
(from left to right):

1. **Pixel group** to configure.
   The valid values are found in the `pixelGroup_t` enumeration:
   `GRP_TELEMETRY`, `GRP_BUTTONS` or `GRP_INDIVIDUAL`.

   > [!NOTE]
   > These three groups are indicative, **not mandatory**.
   > You could use a single LED strip for telemetry,
   > button backlighting and individual pixels.

2. **DOut** pin: GPIO pin number (or alias) where the first pixel
   is connected to the DevKit board.
3. Total **number of pixels** in the RGB LED strip.
   Must be greater than zero.
4. **Logic level shifting**:
   - `false` if your LED strip works in 3.3V logic.
   - `true` if your LED strip works in 5V logic
     (so the ["level shifter"](../../LEDs_en.md)
      is in place).
5. **Pixel driver** (optional).
   The valid values are found in the `pixel_driver_t` enumeration:
   `WS2811`, `WS2812` (default), `WS2815`, `SK6812` or `UCS1903`.
6. **Pixel format** (optional).
    The valid values are found in the `pixel_format_t` enumeration.
    Pass `AUTO` (the default) to use the expected pixel format
    for the given pixel driver.

Example:

```c++
  // 8 pixels for telemetry display in GPIO #8 working in 3.3V logic
  pixels::configure(pixelGroup_t::GRP_TELEMETRY, GPIO_NUM_8, 8 , false);
  // 6 pixels for individual display in GPIO #18 working in 5V logic
  pixels::configure(pixelGroup_t::GRP_INDIVIDUAL, GPIO_NUM_18, 6 , true, pixel_driver_t::WS2815, pixel_format_t::BGR);
```

Place those calls before `hidImplementation::begin()`.
