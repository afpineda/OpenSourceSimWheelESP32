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
7. **Global brightness** (optional).
   A brightness for all pixels.
   The default and recommended value is 255 (maximum brightness).
   Do not set to zero (no brightness).

> [!TIP]
> **Brightness workaround:**
> If a 5V power supply is required but not available for the LED strip,
> use a 3.3V power supply, no level shifter and set the global brightness to 16.
> This is **not optimal** as some colors will be approximate.

Example:

```c++
  // 8 pixels for telemetry display in GPIO #8 working in 3.3V logic
  pixels::configure(pixelGroup_t::GRP_TELEMETRY, GPIO_NUM_8, 8 , false);
  // 6 pixels for individual display in GPIO #18 working in 5V logic
  pixels::configure(pixelGroup_t::GRP_INDIVIDUAL, GPIO_NUM_18, 6 , true, pixel_driver_t::WS2815, pixel_format_t::BGR);
```

Place those calls before `hidImplementation::begin()`.

## Event notifications

If you want to be notified of startup, BLE advertising, connection
or low battery events, place the following call next to `pixels::configure()`:

```c++
notify::begin({new PixelControlNotification()});
```

Events are notified in this way:

- *Startup*: all LEDs light up white for one second.
- *BLE advertising*: all LEDs light up purple until the host computer is connected.
- *Connection*: all LEDS will go out.
- *Low battery*: all LEDs will show a red and blue animation for one second.
- *Bite point calibration*:
  The telemetry segment (exclusively) will briefly show a yellow bar.

## Custom event notifications

If you don't like the predefined notifications and have the skills,
you can customize them:

1. Derive a new class from `PixelControlNotification`.
2. Override the following methods (when required):
   - `pixelControl_OnStart()`
   - `pixelControl_OnBitePoint()`
   - `pixelControl_OnConnected()`
   - `pixelControl_OnBLEdiscovering()`
   - `pixelControl_OnLowBattery()`
3. Place calls to the `pixels` namespace to display custom colors:
   - `pixels::set()`
   - `pixels::setAll()`
   - `pixels::shiftToNext()`
   - `pixels::shiftToPrevious()`
   - `pixels::show()`
4. Create a new instance of your custom class (never to be destroyed).
5. Pass your instance to `notify::begin()`,
   next to `pixels::configure()`.

As an example, see the implementation of [PixelControlNotification](../../../src/common/pixels.cpp).

> [!WARNING]
> Do not call the `pixels` namespace from other UI classes
> (those derived from `AbstractUserInterface`). It will not work.
