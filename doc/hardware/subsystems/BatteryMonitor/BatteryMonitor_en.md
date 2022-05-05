# Battery monitor subsystem

This subsystem is for a **battery-operated** setup. Does not make sense if an external power source is in place.

## Purpose

The purpose of this subsystem is to provide an estimation of how much battery charge is left, so the user knows when to plug the charging cable in. The battery level is a percentage of charge left in the range 0% to 100%. 
Battery level is known to the hosting PC through Bluetooth Low Energy.

## Circuit

This circuit is designed for "1S" Lithium-Polimerer (LiPo) batteries. Some adjustments may be required for other kind of batteries.

### Working principles

A battery monitor needs to read the output voltage of the battery (*not* the powerboost output voltage), this is, the voltage at the positive terminal (the negative terminal is wired to ground (`GND`) at the powerboost module). As the battery gets discharged, that voltage will go down. The output voltage of a LiPo battery is in this range (more or less):

- 4.0V to 5.0V when the powerboost module is charging the battery.
- 3.7V when the battery is fully charged.
- Over 2.4V or less when the battery is discharged. The powerboost module will cut power to avoid over discharge.

In some way, this output voltage should be read through an ADC pin, however, there are two restrictions:

- The output voltage of the battery exceeds 3.3V and would damage the DevKit board, so that voltage has to drop down to a suitable range.
- Reading the output voltage could discharge the battery itself, so the battery monitor should not draw any relevant current. The firmware will read the battery level every few minutes and it takes only a few milliseconds long. This way, battery charge is preserved. The circuit is switched on and off by the means of an NPN-PNP pair.

### Circuit design

The circuit has the following inputs:

- **Battery (+)** pin (battery positive terminal): a wire has to be soldered to that terminal at the powerboost module. 
- **battEN** pin: enables or disables the circuit. Attached to an output-capable GPIO pin at the DevKit board.

And one output:

- **battREAD** pin: provides the current battery level. Attached to an ADC-capable GPIO pin at the DevKit board.

`GND` pin is shared with the main circuit, wired to `GND` at the powerboost module.

![Battery monitor circuit](./BatteryMonitor_falstad.png)

[This simulation at falstad.com](https://falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWcMBMcUHYMGZIA4UA2ATmIxAUgoqoQFMBaMMAKABcQmwVxieHCAFl48eEGAkJhsKFGEEJKmSGCQS80yNjx4peFUPJg4IACZ0AZgEMArgBs2LAOacZIXMIZuZVXywDunPogwQzBfgBOUCG+4BiE0YIYcCxRVGDx0Zg8VEkpUUyZYHyczGIl6fCQqZxC4Ny1whkJlVUBrjweHSDZ4O1UvV6iGDntAsKDdWh4UOyN9fx1wdilnBLGxIQofMSCKEkYBGvQCATYW4TYgnh8CKorxlTm1vaOADKlRSUMKJIi0RBrHYAM50dzQFbVQIMa4xaLhFgAeQBRUETSKs2hbkmExGswAHqVIJ4VgwMMREiAEgAjKxsNh0CIATwAFAAdAAOAEoWISfggjIQkD9IGJCDMmjNafSAKIAOV5VIpYCF4EEyvFIElIGlbAASjKAIIAERYAGUgrEmIRYlQEkDQdEMCw9aUbZxYUxvrDYoJaNFfCcXW6qENOLISTlErkA1Ag67raGVaVvsnfVRsNByLb4yHwAkvWLY373LGJMHE9Efvsy1qMxCy7nK25q8I3OmelnGwgKyrcjNC1qZh2UA2cz2+YNBAXfhAYSsmlQALKGgAaiq8e1cc9+oZQEuQIAAwgALKwRJwASwAdk42cCAPT3o-npxWa+mAD2G+w2GFDX5UMKUXY8zwvOhTHvJ9gRfC8rFMKwfyuGhw1kCgF0PY1L2BABjMCnAgqD72NOhcNfeDEOpVxp1qMRkLqao+S2D16gHCAmgSM1LwAW3sOlCOBXVGSZe8ADdPwcKwCI3DAIAycA0LQLV8xAAA1CS2CsAArOh73Me9pUZABbqx72BHi+K-FggA) shows how it works, but note that the rectangle is not part of the circuit.

Needed parts:

- 100K-ohms resistor (x2)
- 4.7K-ohms resistors (x2) **1% tolerance**. Any impedance in the range 1K-100K ohms will work, as long as both are identical.
- A bipolar junction transistor (x1), NPN type: any kind should work (for example: [BC637](https://www.onsemi.com/pdf/datasheet/bc637-d.pdf)).
- A bipolar junction transistor (x1), PNP type: any kind should work (for example: [BC640](https://www.onsemi.com/pdf/datasheet/bc640-d.pdf)). 

Pay attention to the pinout of your transistors. It *may not match* the one shown here.

Look at this [layout design](./BatteryMonitor.diy) using [DIY Layout Creator](https://github.com/bancika/diy-layout-creator).

![Board design](./BatteryMonitor_diy.png)

## Firmware customization

Customization takes place at file [CustomSetup.ino](../../../../src/Firmware/CustomSetup/CustomSetup.ino).
Ensure the following line of code is in place:

```c
#define ENABLE_BATTERY_MONITOR
```

Locate the line `#define BATTERY_ENABLE_READ_GPIO` and write a GPIO number to the right, where `battEN` is attached to. Locate the line `#define BATTERY_READ_GPIO` and write a GPIO number to the right, where `battREAD` is attached to. For example:

```c
#define BATTERY_ENABLE_READ_GPIO 0
#define BATTERY_READ_GPIO 36
```

Only a rough estimation of battery charge can be provided out of the box. Battery level will be unreliable until the battery is fully charged for the first time.

For accurate battery levels, a battery calibration procedure may be followed, which is extensively documented [here](../../../../src/firmware/BatteryTools/BatteryCalibration/README_en.md) along with the required Arduino sketch.
