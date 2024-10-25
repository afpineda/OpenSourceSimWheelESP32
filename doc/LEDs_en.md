# Working with LEDs

An LED is a special type of diode.
While most diodes have a low forward voltage of around 0.6 volts,
LEDs have a high forward voltage depending on the color of the light:

- Red: 1.6 to 2.0 volts.
- Green: 1.9 to 4.0 volts.
- Blue: 2.5 to 3.7 volts.

An RGB LED is the physical combination of three LEDs: red, green and blue.
As a result, a forward voltage of around 3.7 volts is required to light them.

However, the ESP32 hardware operates at 3.3 volts,
which is not enough to light some of these LEDs.

For this reason, you may need a different power source,
which is labelled `VLed` in this project.
There are a number of options available:

- **5 volts power sources (and higher)**

    Wheel bases and USB cables already operate at 5 volts,
    which is enough to power any LED.
    Attach `VLed` to `5V` if available.

    Higher voltages will work as long as you use a proper
    **current limiting** resistor for each LED.
    When working with **LED strips**,
    make sure your power source is within its specifications.

- **External powerboost module/shield**

    As described in the [power subsystem](./hardware/subsystems/Power/Power_en.md),
    some external power boost modules provide both 5.0 and 3.3 volt outputs.
    Attach `VLed` to the `5V` output.

- **Builtin powerboost module/shield**

    If your DevKit board has built-in battery support,
    there is no 5V output.
    Connect `VLed` directly to the positive pole of the battery.
    This is **not optimal** as some LEDs will go off
    before the battery is discharged.
