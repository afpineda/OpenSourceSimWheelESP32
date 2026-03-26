# Migrating your custom firmware to version 8 from version 7

There are subtle improvements to version 8.
Most custom firmwares from version 7 may work **without changes**.

This is just a summary.
Please refer to the following documentation for more details:

- [Customization guide](./hardware/subsystems/CustomizeHowto_en.md)
- [Firmware customization in the pixel control approach](./hardware/ui/PixelControl_en.md)
- [Firmware customization in the telemetry data approach](./hardware/ui/TelemetryData_en.md)
- [OLED telemetry display](./hardware/ui/MonochromeOLED/MonochromeOLED_en.md)

## Global changes

- The deprecated API regarding rotary coded switches has been removed
  (in the `inputs` namespace).
  Note that there is a newer API for them in the `inputHub` namespace
  already introduced in version 7.
- The limit to 64 *firmware-defined* input numbers has been increased to 128.
  Now, you can use the full 128 range of input numbers anywhere in the firmware.
- The default *input map* works as before, but in modulo 128.
  For example, the input number 45 defaults to 190 when ALT mode is engaged and
  the input number 95 defaults to 31 when ALT mode is engaged.
- Your existing custom firmware does not require any change regarding input
  numbers.

## Pixel control

The parameters to `pixels::configure()` have changed.
The `PixelFormat` parameter has been removed and there is a new parameter
that allows physical mounting of the LED strip in right-to-left layout.

## Input routing

This is a new feature that allows a button press
to be routed to the user interface,
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

The monochrome OLED telemetry display takes advantage
of this feature (optional).

## Monochrome OLED displays

There are new constructor parameters to `OledTelemetry128x64`,
but it remains compatible with version 7:

- There are three dashboards to choose from:
  standard, alternate and "battery level"
  (available only on battery-operated systems).
- You can choose the dashboard in a constructor parameter.
- If you use the *input routing* feature,
  you can choose another dashboard at anytime via button press.
- Future versions may introduce new dashboards.
