# Overview of telemetry data

This firmware is able to receive and process telemetry data sent by the host computer.
This is a different approach to telemetry display than commercial products,
where the host computer controls the display hardware.
This is the only way for the do-it-yourself enthusiast
to build their own display hardware using a single universal firmware.
The downside is that there is little scope for user customization.

## Available telemetry data

The `telemetryData_t` type defined in [SimWheelTypes.h](../src/include/SimWheelTypes.h)
describes the whole set of telemetry data.
It is divided into four groups:

- Powertrain
- ECU (Electronic Control Unit)
- Race control
- Gauges

## Limitations

There are certain limitations to what the ESP32 hardware can do to display telemetry data:

- **CPU power**: this firmware prioritizes functionality over telemetry display.
  Just the remaining CPU power is used for this.
- **Bandwidth**: a telemetry display needs to be updated quickly.
  This uses a lot of bandwidth,
  which is limited to 1Mbps in the BLE technology.
  This firmware announces the maximum frames per second capability,
  and the SimHub plugin adjusts the transfer rate accordingly.
- **I/O capabilities**: DevKit boards cannot achieve high throughput.
  For example, a 128x64 monochrome display, in the I2C bus,
  is unable to display more than 20 frames per second.

For this reason, telemetry displays are **limited** to "rev lights",
LED segment displays, real car gauges and the like.

**This firmware cannot integrate high-performance displays.**
If this is your wish,
integrate them as a separate device in your sim-wheel or button box.
