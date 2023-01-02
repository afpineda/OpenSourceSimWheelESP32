# Battery monitor subsystem

This subsystem is for a **battery-operated** setup. Does not make sense if an external power source is in place.

## Purpose

The purpose of this subsystem is to provide an estimation of how much battery charge is left, so the user knows when to plug the charging cable in. The battery level is a percentage of charge left in the range 0% to 100%. 
Battery level is known to the hosting PC through Bluetooth Low Energy.

There are two alternative implementations to choose from:

- Battery monitor: recommended since it avoids excesive battery drainage.
- Simple voltage divider: can be found in some ESP32 devkit boards as a built-in feature. It drains current at all times.

## Working principles

A battery monitor needs to read the output voltage of the battery (*not* the powerboost output voltage), this is, the voltage at the positive terminal (the negative terminal is wired to ground (`GND`) at the powerboost module). As the battery gets discharged, that voltage will go down. The output voltage of a LiPo battery is in this range (more or less):

- 4.0V to 5.0V when the powerboost module is charging the battery.
- 3.7V when the battery is fully charged.
- Over 2.4V or less when the battery is discharged. The powerboost module will cut power to avoid over discharge.

In some way, this output voltage should be read through an ADC pin, however, there are two restrictions:

- The output voltage of the battery exceeds 3.3V and would damage the DevKit board, so that voltage has to drop down to a suitable range.
- Reading the output voltage could discharge the battery itself, so the battery monitor should not draw any relevant current. 

The firmware will read the battery level every few minutes and it takes only a few milliseconds long.

## Circuit: battery monitor

This circuit is designed for "1S" Lithium-Polimerer (LiPo) batteries. Some adjustments may be required for other kind of batteries. The circuit is switched on and off by the means of an NPN-PNP pair. It will not draw any relevant current except for a few milliseconds every few minutes.

### Design (1)

The circuit has the following inputs:

- **Battery (+)** pin (battery positive terminal): a wire has to be soldered to that terminal at the powerboost module.
- **battEN** pin: enables or disables the circuit. Attached to an output-capable GPIO pin at the DevKit board.

And one output:

- **battREAD** pin: provides the current battery level. Attached to an ADC-capable GPIO pin at the DevKit board.

`GND` pin is shared with the main circuit, wired to `GND` at the powerboost module.

![Battery monitor circuit](./BatteryMonitor_falstad.png)

[This simulation at falstad.com](https://falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWcMBMcUHYMGZIA4UA2ATmIxAUgoqoQFMBaMMAKABcQmwVxieHCAFl48eEGJTAE8g4pDQJshLFGiKEmfJG4oEgsJGLIqAEzoAzAIYBXADZsWAc07YeuYQ1chXVXywDunHhUwUG+UCwATlAgoVRgGIQxghhwUTEJSVSYPFQpadFMiSKczGJ8GfAFnELg3DXCmZVVAS5ukB5eOeCt2Rj8Xf0RgQLC3aMgaHgRHBNlDbFU2KWcEgZ4OPJggpAIxNgeEgTYlMRSBpDYUo1wIGZWdg4AMqXFYBUMuknvueAgVrYAM50bzQZaQVoMQTTOKLCIAeQyb0EjTewzakyGE26EIAHqUOi5OBgjHkQEkAEaWNhsOiRACeAAoADoABwAlCx8Z8EOQwIQkJ95OBCNNGtMqTSAKIAOS55KM-KQ20VopA4pAkrYACUpQBBAAiLAAymEYkxCOEqEkAcCYhgWNrSpbONDSh83eEdtQrWpHc6qJ5+CgUJ1fnkyb6EP6LYH+e7+PGvUtoOQozH+fEkkwKknkksYr4-U7Y+aQ8IreqU+DC8WA95g6GG4Wq5NUy2JBmXW6czxPfm2zWi9HueNBNndBAocsbiAALJ6gAa8s8ggGU90gZQYuQIAAwgALSyRRwASwAdo5mYCAPTXvfHxyWc8mAD2K+wJ1KwconCMs8PR86BMa870BB8T0sExLA-A4aE4EMeEUdVdwNU9AQAYyPE9gNA68DToTDHygmCKRcccajEODajxSjXTqaYmBQ+NjVPABbOxqVwwEtTpelrwAN1fexLEcOgVwwCAEnAENJjJeMADVhLYSwACs6GvMxr0lOkAFvLGvQF2M4t8WCAA) shows how it works, but note that the rectangle is not part of the circuit.

Needed parts:

- 100K-ohms resistor (x2)
- 4.7K-ohms resistors (x2) **1% tolerance**. Any impedance in the range 1K-100K ohms will work, as long as both are identical.
- A bipolar junction transistor (x1), NPN type: any kind should work (for example: [BC637](https://www.onsemi.com/pdf/datasheet/bc637-d.pdf)).
- A bipolar junction transistor (x1), PNP type: any kind should work (for example: [BC640](https://www.onsemi.com/pdf/datasheet/bc640-d.pdf)). 

Pay attention to the pinout of your transistors. It *may not match* the one shown here.

Look at this [layout design](./BatteryMonitor.diy) using [DIY Layout Creator](https://github.com/bancika/diy-layout-creator).

![Board design](./BatteryMonitor_diy.png)

## Circuit: simple voltage divider

This kind of circuit is built into some ESP32 boards. For example:

- [Tinypico](https://www.tinypico.com/)
- [Adafruit Feather 32u4 Bluefruit LE](https://www.adafruit.com/product/2829)
- [Wemos D32 boards](https://www.wemos.cc/en/latest/d32/d32.html)

In such a case, there is no need to build this subsystem. However, we also provide the design in case you are short of available GPIO pins for the previous alternative.

### Design (2)

The circuit has the same inputs and outputs as the previous alternative, except for `BattEN`, which is not needed.

- **Battery (+)** pin (battery positive terminal). Not exposed and not needed in the alluded boards.
- **battREAD** pin: provides the current battery level. May be exposed or not in the alluded boards. If not exposed, it will be wired internally to a certain GPIO. Some times tagged as `BAT` or `VBAT`.

![Voltage divider for the battery monitor](./BatteryDivider_falstad.png)

[This simulation at falstad.com](https://falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWcMBMcUHYMGZIA4UA2ATmIxAUgoqoQFMBaMMAKAHMQHsURcAWTt15ooUFgHdOeKtKlV5LAE6jZVMBkKiw8OEq0bRmHmp2QJgnvwsgj4c1Vtcets5IazVIVSwDy+zWB8AuoBYgAenNoCXJwYxKICmgBGAIYALml0igCeABQAOgAOAJQsEUyYyHicfNXMEMHVAELpmTkFJWUgJOCESIHxYITVjSCpGQBKAKIAggAiLADKcqJMhPKimgBmKQA2AM50ohgsE5HrNdVMxDwMtaJUfLQPUNAIp+dUTpwoKNFCGyeIAEGxg7zOay+Q0iN3OLyB2Gg5FBbw+kN6MJ40MBVGwLzBaKGGwYvxB8Nx0DxKPBn2EPz+dJxNiR+NREKJwKuYFh9yZKEprPe5UcfE0JIQEDueOCVAAsjMABpdLh8W7YSUoSg-EbIEAAYQAFilFGwAJYAOzY+X2AHprXrjWwUuaACYAe2V2GwSAqPy1DHiMv1RpNdBd1rt+wdJpSLpSnuwAn9vx4CGlurmpv2AGMQ2wwxHrXM6DnHbH40lBKLOIQsYnuiDlbWauAwFzgRjFqaALYAV126QL+3GbWy1oAbm7dmkUvnlRgIOpwL8bI8MQA1KczgBWdGtLr3w8HigAtylrfse-3Yx6gA) shows how it works, but note that the rectangle is not part of the circuit.

Needed parts:

- 100K-ohms resistor (x2), 1% tolerance.

## Firmware customization

Only a rough estimation of battery charge can be provided out of the box. Battery level will be unreliable until the battery is fully charged for the first time.

For accurate battery levels, a battery calibration procedure may be followed, which is extensively documented [here](../../../../src/Firmware/BatteryTools/BatteryCalibration/README_en.md) along with the required Arduino sketch.

Customization takes place at file [CustomSetup.ino](../../../../src/Firmware/CustomSetup/CustomSetup.ino).
Ensure the following line of code is in place:

```c
#define ENABLE_BATTERY_MONITOR
```

### Battery monitor (design 1)

Locate the line `#define BATTERY_ENABLE_READ_GPIO` and write a GPIO number to the right, where `battEN` is attached to. Locate the line `#define BATTERY_READ_GPIO` and write a GPIO number to the right, where `battREAD` is attached to. For example:

```c
#define BATTERY_ENABLE_READ_GPIO 0
#define BATTERY_READ_GPIO 36
```

### Simple voltage divider (design 2)

Locate the line `#define BATTERY_READ_GPIO` and write a GPIO number to the right, where `battREAD` is attached to. Set `BATTERY_ENABLE_READ_GPIO` as shown below:

```c
#define BATTERY_ENABLE_READ_GPIO -1
#define BATTERY_READ_GPIO 36
```
