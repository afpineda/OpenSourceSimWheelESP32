# Firmware customization in the *telemetry data* approach

Some tweaks are required.

1. Add the following header to the top of your sketch file:

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
   call `i2c::begin()` first. The parameters are:
   - 1st: the `SDA` pin for the secondary bus.
   - 2nd: the `SCL` pin for the secondary bus.
   - 3rd: set to `true`. This is mandatory.

   Both pins **must** support input, output and pull-up resistors.
   For example:

   ```c++
   i2c::begin(GPIO_NUM_19,GPIO_NUM_20,true);
   ```

4. Place a call to `notify::begin()` at the beginning of `setup()`.
   The parameters are:
   - 1st: a list, in brackets, of all the instances previously created.
   - 2nd: a display rate, in frames per second.
     Please adapt this to the capabilities of your display hardware.
   - 3rd (optional): a stack size in bytes for the frame server.
     The default stack size is 4 KB.
     Increase this if you are getting "stack canary" or "dual exception" errors and reboots.
     A stack size smaller than 2 KB may not work.

     For example:

     ```c++
     notify::begin({ui1},50,5000);
      ```

A better example follows:

```c++
...

#include "SimWheel.h"
#include "SimWheelUI.h"

...

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);

    /// --- relevant code here ---
    auto ui1 = new SimpleShiftLight(GPIO_NUM40);
    notify::begin({ui1},50);
    /// --- relevant code here ---

    power::begin((gpio_num_t)WAKE_UP_PIN);

    power::setPowerLatch(
        (gpio_num_t)POWER_LATCH,
        LATCH_MODE,
        LATCH_POWEROFF_DELAY);

    userSettings::begin();
    simWheelSetup();
    hidImplementation::begin(
        DEVICE_NAME,
        DEVICE_MANUFACTURER
    );

    batteryMonitor::begin();
    inputs::start();
}

...
```

## How to design your own telemetry display

If you have the skills,
you can design your own display hardware and
then write the code to make it work.
Consider [contributing](/.github/CONTRIBUTING.md) to this project.

1. Place your ".h" or ".hpp" files in the "src/include" folder.
2. Place your ".cpp" files in the "src/common" folder.
3. Add them to the "includes.txt" file in your sketch folder.
4. Run the [sources setup procedure](../../firmware/sourcesSetup_en.md)
   again.
5. Include the header file
   [SimWheelTypes.h](../../../src/include/SimWheelTypes.h)
6. When using the I2C bus, include the header file
   [i2cTools.h](../../../src/include/i2cTools.h).
7. Derive a new C++ class from `AbstractUserInterface`.
   See the [documentation](../../../src/include/SimWheelTypes.h)
   for this class.
8. In the class constructor, make sure that at least one of the
   following inherited fields is set to true:
   - `requiresPowertrainTelemetry`
   - `requiresECUTelemetry`
   - `requiresRaceControlTelemetry`
   - `requiresGaugeTelemetry`
9. When using the I2C bus, in the class constructor
   place a call to `i2c::require()` to ensure the
   bus is initialized.
10. Override `AbstractUserInterface::onTelemetryData()`
    to react to new telemetry data.
11. Override `AbstractUserInterface::serveSingleFrame()`
    to handle the display hardware on a timed basis.

See [SimwheelUI.h](../../../src/include/SimWheelUI.h)
and [SimWheelUI.cpp](../../../src/common/SimWheelUI.cpp)
for examples.

### Available telemetry data

The `telemetryData_t` type defined in [SimWheelTypes.h](../../../src/include/SimWheelTypes.h)
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
- [RGB LED strips](./LEDStrips/LEDStrips_en.md). Including:
  - "Rev lights"
  - Shift light
  - Race flags
  - Witnesses
