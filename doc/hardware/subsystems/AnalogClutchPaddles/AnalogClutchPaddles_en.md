# Subsystem for analog clutch paddles

This subsystem is optional.
Take into account that this subsystem is not "battery-friendly".

## Purpose

To provide two **potentiometers** as analog inputs to the system.
The position of each potentiometer is translated into a logical axis position,
which the user can map to several functions:

- Regular buttons: on/off.
- Two autonomous axes:
  this way, each clutch paddle may work as a regular clutch,
  a throttle, a brake or any other input.
- F1-style clutch:
  the position of both axes are combined into a single axis.
- Launch control:
  close to the F1-style clutch, but there is a master paddle.
- "ALT" buttons.

This subsystem requires two ADC pins.
If not available, potentiometers may still be used as digital clutch paddles as shown in the
[switches subsystem](../Switches/Switches_en.md).

## Circuit design

There is no circuit involved, just wiring:

![Analog clutch wiring](./AnalogClutchWiring.png)

Notes:

- An ADC-capable pin is required for each clutch paddle.
- `3V3` and `GND` are interchangeable.
  If the clutch (or axis) goes to 100% when idle, swap those terminals.
  If you don't have the chance to swap those terminals,
  **the companion app has the ability to swap axis polarity by user request**.
- An high impedance potentiometer is advisable (10 K-ohms or more).
  Potentiometers will drain current at all times, which is bad for batteries.

## Autocalibration

By default, both potentiometers are expected to work in the full range of voltage.
Many times, this is not the case due to a physical limit on the rotation of a potentiometer.
In such a case, the user should ask for "recalibration".
Once both potentiometers are moved from end to end,
the actual ranges of voltage will be noted and saved to flash memory after a short delay.

## Firmware customization

Customization takes place at file [CustomSetup.ino](../../../../src/Firmware/CustomSetup/CustomSetup.ino).
Place a single call to `inputs::setAnalogClutchPaddles()` inside `simWheelSetup()`:

- First parameter is the ADC pin number for the left clutch paddle.
- Second parameter is the ADC pin number for the right clutch paddle.

For example:

```c
void simWheelSetup()
{
    ...
    inputs::setAnalogClutchPaddles(GPIO_NUM_12, GPIO_NUM_13);
    ...
}
```

You should also set two input numbers for the clutch paddles to work in "regular buttons" mode:
place a call to `inputHub::clutch::inputs()`.
More on this later.

Recalibration can be achieved through the companion app,
but you may assign a button combination for that.
Place a call to `inputHub::clutch::cmdRecalibrateAxisInputs()`.
The parameter is a sequence of input numbers between brackets.
All inputs must be active at the same time, and none of the others.

The following example will set the combination of buttons `A` and `Start` for recalibration:

```c
void simWheelSetup()
{
    ...
    inputHub::clutch::cmdRecalibrateAxisInputs({JOY_START,JOY_A});
    ...
}
```
