# Firmware customization in the *telemetry data* approach

Some out-of-the-box telemetry displays are available.
Follow the links for an in-depth explanation:

- [Simple shift light](./SimpleShiftLight/SimpleShiftLight_en.md)

  One or two single-color LEDS to display when to shift gears.

- [PFC8574-driven "rev lights"](./PCF8574RevLights/PCF8574RevLights_en.md)

  A display composed by eight single-color LEDS and
  a PFC8574 GPIO expander chip.

- [Monochrome OLED 128x64 telemetry display](./MonochromeOLED/MonochromeOLED_en.md)

  A monochrome OLED with 128x64 pixels showing RPM bar,
  gear, TC, ABS, brake bias, fuel and pit limiter alerts.

Any telemetry display is potentially able to display notifications
(such as connection or low battery),
but it depends on the particular implementation.

As a general rule, each telemetry display is represented by a C++ class.
You enable the hardware by calling
`ui::add<ClassName>( constructor parameters );`.

## Input routing

This feature allows a button press to be routed to the user interface,
typically to select another dashboard.
For this to work your custom firmware must meet two conditions:

1. Place as many calls as you need to
   `inputHub::route_to_ui::add()`.
   Pass a valid input number as parameter.
   This input number must be assigned to input hardware via
   the `inputs::add*` API.
   That input number is said to be *routed*.
2. Where available, pass the routed input number to
   the proper constructor parameter in `ui::add<ClassName>()`.

> [!CAUTION]
> Once an input number is routed,
> it counts as **not assigned** to the input hardware.
> So, you will get an error message if you try to use them in other
> firmware features.
>
> It does not work well with non-momentary switches.

Example:

```c++
...
inputs::addButton(GPIO_NUM_26, 120); // Assign 120 to input hardware
inputHub::route_to_ui:add(120); // Route 120 to UI instances
...
OLEDParameters my_oled_params;
...
// Create UI instance
ui::add<OledTelemetry128x64>(
   my_oled_params,
   I2CBus::SECONDARY,
   true,
   OledDashboard::_DEFAULT,
   120); // Use 120 to select the next dashboard
...
```

## How to design your own telemetry display

If you have the skills,
you can design your own display hardware and
then write the code to make it work.
Consider [contributing](../../../.github/CONTRIBUTING.md) to this project.

1. Place your ".hpp" files in the "src/include" folder.
2. Place your ".cpp" files in the "src/common" folder.
3. Add your ".cpp" files (only) to the "includes.txt" file in your sketch folder.
   File names are case-sensitive.
4. Run the [sources setup procedure](../../firmware/sourcesSetup_en.md)
   again.
5. Include the header file
   [SimWheelTypes.hpp](../../../src/include/SimWheelTypes.hpp)
6. When using the I2C bus, include the header file
   [HAL.hpp](../../../src/include/HAL.hpp).
7. Derive a new C++ class from `AbstractUserInterface`.
   See the documentation for this class.
8. In the class constructor, make sure that at least one of the
   following inherited fields is set to `true`:
   - `requiresPowertrainTelemetry`
   - `requiresECUTelemetry`
   - `requiresRaceControlTelemetry`
   - `requiresGaugeTelemetry`
9. When using the I2C bus, in the class constructor
   place a call to `internals::hal::i2c::require()` to ensure the
   bus is initialized.
10. Override `AbstractUserInterface::getMaxFPS()` to
    announce your device capabilities to the *frameserver*.
11. Override `AbstractUserInterface::getStackSize()` to
    announce the required stack size (in bytes).
    If you get "stack canary" or "dual exception" errors,
    increase the stack size.
    Do not set too high, since memory is limited.
12. Override `AbstractUserInterface::onTelemetryData()`
    to react to new telemetry data.
13. Override `AbstractUserInterface::serveSingleFrame()`
    to handle the display hardware on a timed basis.
14. Override `AbstractUserInterface::onUserInput()`
    to react to *routed* inputs.
    This gets called on button release.
15. Place the required `#include` directives in your sketch file.
16. Create a single instance of your custom user interface class
    by calling `ui::add<YourClass>( constructor parameters );`.

See [SimWheelUI.hpp](../../../src/include/SimWheelUI.hpp),
[SimWheelUI.cpp](../../../src/common/SimWheelUI.cpp) and
[SimWheelUI_gfx.cpp](../../../src/common/SimWheelUI_gfx.cpp)
for examples.

### Available telemetry data

The `TelemetryData` type defined in
[SimWheelTypes.hpp](../../../src/include/SimWheelTypes.hpp)
describes the whole set of telemetry data.
It is divided into four groups:

- Powertrain
- ECU (Electronic Control Unit)
- Race control
- Gauges
