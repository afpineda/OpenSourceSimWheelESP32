# Overview of telemetry

There are **two non-exclusive approaches** to telemetry in this project:

- The **telemetry data** approach:

  This firmware is able to receive and process raw telemetry data sent from the host computer.

  - You can have as many telemetry displays as the ESP32 hardware can handle.
  - The display hardware is not limited by the firmware.
    Anything can be implemented: real car gauges, LEDs, speakers, etc.
  - The display hardware can also notify the user of some events,
    such as low battery.
  - Your custom firmware determines how telemetry data is displayed.
    As a result, there is no room for end-user configuration.

- The **pixel control** approach:

  This firmware is capable of receiving and displaying pixel color data sent from the host computer.

  - The display hardware is limited to three single wire RGB LED strips:
    *telemetry LEDs*, *button backlighting* and *individual LEDs*.
  - Optionally, the display hardware can also notify the user of some events,
    such as low battery.
  - The host computer determines how the telemetry data is displayed.
    This allows for end-user configuration to be carried out.

You can mix both approaches in the same custom firmware
**as long as** they don't share display hardware.
You will get unexpected behavior if you use the same
display hardware for telemetry and pixel control.

## Limitations

There are certain limitations to what the ESP32 hardware can do to display telemetry or pixel data:

- **CPU power**:
  In the *telemetry data* approach,
  this firmware prioritizes functionality over telemetry display.
  Just the remaining CPU power is used for this.
  This is not the case in the *pixel control* approach.
- **Bandwidth**: a telemetry display needs to be updated quickly.
  This uses a lot of bandwidth,
  which is limited to 1Mbps in the BLE technology.
  For raw telemetry data,
  this firmware announces the maximum frames per second capability,
  and the SimHub plugin adjusts the transfer rate accordingly.
  Unnecessary data will not be sent.
- **I/O capabilities**: DevKit boards cannot achieve high throughput.
  For example, a 128x64 monochrome display, in the I2C bus,
  is unable to display more than 20 frames per second.
- **Memory size**:
  The ESP32 architecture has 320 KB of
  [RAM](https://en.wikipedia.org/wiki/Random-access_memory).
  A full color display, 640x400 pixels,
  requires 768 KB for a single image.

For this reason, telemetry displays are **limited** to
RGB LED strips, segment displays, real car gauges and the like.

**This firmware cannot integrate high-performance displays.**
If this is your wish,
integrate them as a separate device in your sim-wheel or button box.

## SimHub plugin

In order to receive telemetry and pixel data,
you must install [SimHub](https://simhubdash.com) in your PC
and a custom plugin which is hosted here:

[https://github.com/afpineda/SimWheelESP32-SimHub-Plugin](https://github.com/afpineda/SimWheelESP32-SimHub-Plugin)

### Display hardware using the I2C bus

Devices that use the primary I2C for input must not use this bus for display.
Otherwise performance problems may occur.

A secondary I2C bus must be used, which requires manual initialization.
Note that the secondary I2C bus is **not available on ESP32C3 boards** (sorry).

If your display hardware requires the secondary I2C bus,
call `i2c::begin()` first. The parameters are:
- 1st: the `SDA` pin for the secondary bus.
- 2nd: the `SCL` pin for the secondary bus.
- 3rd: set to `true`. This is mandatory.

Both pins **must** support input, output and pull-up resistors.
For example:

```c++
#include "SimWheelUI.h"

...

i2c::begin(GPIO_NUM_19,GPIO_NUM_20,true);
```
