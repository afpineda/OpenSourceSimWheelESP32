# Firmware customization in the *telemetry data* approach

Some tweaks are required.

1. Ensure the following header in in the top of your sketch file:

   ```c++
   #include "SimwheelUI.h"
   ```

2. Create an instance of each display hardware
   (you can have more than one) using the `new` operator.
   Each display hardware is represented by a C++ class.
   The classes available are explained later. For example:

   ```c++
   auto ui1 = new SimpleShiftLight(GPIO_NUM40);
   ```

3. If your display hardware requires the secondary I2C bus,
   call `inputs::initializeI2C()` first.

4. Place a call to `ui::add()` at the beginning of `setup()`.
   Pass the previous instance as parameter.
   You can place as many calls as required.

     For example:

     ```c++
     ui::add(ui1);
      ```

## How to design your own telemetry display

If you have the skills,
you can design your own display hardware and
then write the code to make it work.
Consider [contributing](../../../.github/CONTRIBUTING.md) to this project.

1. Place your ".h" or ".hpp" files in the "src/include" folder.
2. Place your ".cpp" files in the "src/common" folder.
3. Add ".cpp" files to the "includes.txt" file in your sketch folder.
4. Run the [sources setup procedure](../../firmware/sourcesSetup_en.md)
   again.
5. Include the header file
   [SimWheelTypes.hpp](../../../src/include/SimWheelTypes.hpp)
6. When using the I2C bus, include the header file
   [HAL.hpp](../../../src/include/HAL.hpp).
7. Derive a new C++ class from `AbstractUserInterface`.
   See the documentation for this class.
8. In the class constructor, make sure that at least one of the
   following inherited fields is set to true:
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

See [SimwheelUI.hpp](../../../src/include/SimWheelUI.hpp)
and [SimWheelUI.cpp](../../../src/common/SimWheelUI.cpp)
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

## Available displays for raw telemetry data

The C++ classes already included in this project (representing display hardware)
are explained in the following documents.

- [Simple shift light](./SimpleShiftLight/SimpleShiftLight_en.md)
- [PFC8574-driven "rev lights"](./PCF8574RevLights/PCF8574RevLights_en.md)
