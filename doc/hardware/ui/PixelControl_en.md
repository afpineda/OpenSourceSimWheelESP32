# Firmware customization in the *pixel control* approach

Place up to three calls to `pixels::configure()` with the following parameters
(from left to right):

1. **Pixel group** to configure.
   The valid values are found in the `PixelGroup` enumeration:
   `GRP_TELEMETRY`, `GRP_BUTTONS` or `GRP_INDIVIDUAL`.

   > These three groups are indicative, **not mandatory**.
   > For example, you could use a single LED strip for telemetry,
   > button backlighting and individual pixels,
   > all at the same time in the `GRP_TELEMETRY` group.

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
   The valid values are found in the `PixelDriver` enumeration:
   `WS2811`, `WS2812` (default), `WS2815`, `SK6812` or `UCS1903`.
6. **Pixel format** (optional).
    The valid values are found in the `PixelFormat` enumeration.
    Pass `AUTO` (the default) to use the expected pixel format
    for the given pixel driver.
7. **Global brightness** (optional).
   A brightness for all pixels.
   The default and recommended value is 255 (maximum brightness).
   Do not set to zero (no brightness).

## Brightness workaround

If a 5V power supply is required but not available for the LED strip,
use a 3.3V power supply, no level shifter and set the global brightness to **128** or lower.
This is **not optimal** as some colors will be approximate, but it works pretty well.

## Example

```c++
  // 8 pixels for telemetry display in GPIO #8 working in 3.3V logic
  pixels::configure(PixelGroup::GRP_TELEMETRY, GPIO_NUM_8, 8 , false);
  // 6 pixels for individual display in GPIO #18 working in 5V logic
  pixels::configure(PixelGroup::GRP_INDIVIDUAL, GPIO_NUM_18, 6 , true, PixelDriver::WS2815, PixelFormat::BGR);
```

## Event notifications

If you want to be notified of startup, BLE advertising, connection
or low battery events, place the following call next to `pixels::configure()`:

```c++
ui::addPixelControlNotifications();
```

Events are notified in this way:

- *Startup* (shown for one second and a half)

  - If there is no battery all the LEDs light up white.
  - If there is a battery, the state of charge is shown as follows:
    - Telemetry group: the length of a light-blue bar reflects the battery level.
    - Other groups: a color gradient reflects the battery level in all pixels,
      bright green for 100%, bright red for 0%.

- *BLE advertising*: all the LEDs light up purple until the host computer is connected.
- *Connection*: all the LEDs will go out.
- *Low battery*: all the LEDs will show a red and blue animation for one second.
- *Bite point calibration*:
  the telemetry segment (exclusively) will briefly show a yellow bar.
- *Saved settings*: all the LEDs will flash green twice.

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
   - `pixelControl_OnSaveSettings()`
3. Use the following inherited methods to display custom colors:
   - `set()`
   - `setAll()`
   - `shiftToNext()`
   - `shiftToPrevious()`
   - `show()`
4. Create a new instance of your custom class (never to be destroyed).
5. Pass a single instance to `ui::add()` or create one on-the-fly
   using the syntax `ui::add<YourClassName>( constructor parameters )`.

As an example, see the implementation of [PixelControlNotification](../../../src/common/pixels.cpp).
