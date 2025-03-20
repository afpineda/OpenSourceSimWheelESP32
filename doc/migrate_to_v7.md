# Migrating your custom firmware to version 7 from a previous version

There are important improvements starting with version 7.
Unfortunately, any custom firmware will need to be rewritten.

Please refer to the following documentation for more details:

- [Customization guide](./hardware/subsystems/CustomizeHowto_en.md)
- [Switches subsystem](./hardware/subsystems/Switches/Switches_en.md)
- [Power subsystem](./hardware/subsystems/Power/Power_en.md)
- [Battery monitor subsystem](./hardware/subsystems/BatteryMonitor/BatteryMonitor_en.md)

## Global changes

- The "public" API is limited to what you need
  to customize the firmware.
  Internal details are hidden by default.
- Error reporting does not require the "Core debug level" setting,
  but remember to set the serial monitor to 115200 bauds.
- The firmware will do **more checks** and will fail earlier
  if something is not configured correctly
  in your custom firmware.
- You are no longer forced to declare global variables
  such as GPIO pins and input number combinations.
  Please, **declare them as local** to avoid unnecessary memory
  usage.
- Some data type names ending in `_t` have been renamed (case sensitive).

  | Before                   | Now                    |
  | ------------------------ | ---------------------- |
  | gpio_num_array_t         | (see note 1)           |
  | inputNumber_t            | InputNumber            |
  | inputNumberCombination_t | InputNumberCombination |
  | MCP23017_pin_t           | MCP23017Pin            |
  | mux16_pin_t              | Mux16Pin               |
  | mux32_pin_t              | Mux32Pin               |
  | mux8_pin_t               | Mux8Pin                |
  | PCF8574_pin_t            | PCF8574Pin             |
  | pixelDriver_t            | PixelDriver            |
  | pixelFormat_t            | PixelFormat            |
  | pixelGroup_t             | PixelGroup             |
  | powerLatchMode_t         | PowerLatchMode         |
  | revLightsMode_t          | RevLightsMode          |
  | sr8_pin_t                | SR8Pin                 |
  | telemetryData_t          | TelemetryData          |

  - *Note 1:*

    GPIO arrays has been substituted by specialized types:
    `GPIOCollection`, `InputGPIOCollection` and `OutputGPIOCollection`.
    There is no need to declare variables of these types
    as you can initialize them using GPIO numbers between braces.

- The telemetry display using RGB LED strips has been **removed**
  as pixel control is a better option.

## Firmware input numbers (`inputs` namespace)

You have to declare input hardware in a different,
but more intuitive way.

### Single button

`inputs::addDigital()` has been renamed to `inputs::addButton()`.
Same parameters.

### Button matrices

Now button matrices are declared as... (C++) matrices.
For example:

```c++
void simWheelSetup()
{
    ButtonMatrix mtx;
    mtx[12][14] = 1;
    mtx[12][15] = 2;
    mtx[13][14] = 4;
    mtx[13][15] = 6;
    inputs::addButtonMatrix(mtx);
}
```

Where `12` and `13` are **selector pins**,
`14` and `15` are **input pins**, and
`1`, `2`, `4` and `6` are **input numbers**.
You can also use aliases.

You can have a button matrix that works in negative logic,
but the hardware design is not provided in this project.
Simply call `inputs::addButtonMatrix(mtx, true)`.
This allows you to reuse hardware from another project.

For convenience, you can call `populateButtonMatrix()`
to assign input numbers on a sequential basis.
For example:

```c++
void simWheelSetup()
{
    ButtonMatrix mtx;
    populateButtonMatrix(mtx, {12,13}, {14,15}, 1);
    inputs::addButtonMatrix(mtx);
}
```

Where `12` and `13` are **selector pins**,
`14` and `15` are **input pins**, and
`1` is the first input number.
This is equivalent to:

```c++
mtx[12][14] = 1;
mtx[12][15] = 2;
mtx[13][14] = 3;
mtx[13][15] = 4;
````

### Analog multiplexers

1. Declare each multiplexer chip,
   passing the **input pin** as a constructor parameter.
   Use the classes
   `AnalogMultiplexerChip8`, `AnalogMultiplexerChip16` or `AnalogMultiplexerChip32`
   for 8, 16 and 32 channel chips respectively.
   For example: `AnalogMultiplexerChip8 chip1(GPIO_NUM_10);`
2. Assign input numbers to pin tags on each chip using the array syntax.
   For example: `chip1[Mux8Pin::A0] = 1;`.
   Qualify each pin tag with `Mux8Pin::`, `Mux16Pin::` or `Mux32Pin::`
   for 8, 16 and 32 channel chips respectively.
3. Call `inputs::addAnalogMultiplexerGroup()`:
   - The first 3, 4 or 5 parameters are **selector pins**,
     depending on the number of channels
     (8, 16 or 32 channels).
   - You cannot mix 8, 16 and 32 channel chips.
     There is no room for error.
   - In the last parameter, group the chip instances
     between braces.

For example:

```c++
void simWheelSetup()
{
    AnalogMultiplexerChip8 chip1(13);
    AnalogMultiplexerChip8 chip2(14);
    chip1[Mux8Pin::A0] = 1;
    ...
    chip2[Mux8Pin::A0] = 8;
    ...
    inputs::addAnalogMultiplexerGroup(10,11,12,{chip1,chip2});
}
```

Where `13` and `14` are **input pins**,
`Mux8pin::A<n>` are **chip tags**,
`1`-`8` are **input numbers**, and
`10`, `11`, `12` are **selector pins**.

### GPIO expanders

- Declare each GPIO expander chip using the
  classes `MCP23017Expander` or `PCF8574Expander`.
- Assign an input number to each tag.
- Call `inputs::addMCP23017Expander()` or `inputs::addPCF8574Expander()`.
  They require the same parameters (from left to right):
  - A chip instance,
  - An I2C address (full or hardware).
  - Optionally, `true` for a full address, or `false` for a hardware address.
  - Optionally, an I2C bus.

For example:

```c++
void simWheelSetup()
{
    PCF8574Expander chip1;
    chip1[PCF8574Pin::P0] = 1;
    ...
    inputs::addPCF8574Expander(chip1, 0);
}
```

Where `PCF8574Pin::P<n>` is a chip tag,
`1` is an input number, and
`0` is a hardware address.

### PISO shift registers

- Declare each PISO chip using the class `ShiftRegisterChip`.
- Assign an input number to each tag.
- Group these chips between braces.
  The position of each chip in the group is the position in the chain.
  The left-most instance is the first one.
- Call `inputs::add74HC165NChain()`. Parameters:
  - `Load` pin.
  - `Next` pin.
  - `Input` pin.
  - The group of chip instances.
  - Optionally, the input number assigned to the `SER`
    tag in the last chip in the chain.

For example:

```c++
void simWheelSetup()
{
    ShiftRegisterChip chip1, chip2;
    chip1[SR8Pin::A] = 1;
    chip1[SR8Pin::B] = 2;
    ...
    chip2[SR8Pin::A] = 8;
    ...
    inputs::add74HC165NChain(12,13,14, {chip1,chip2}, 55);
}
```

Where `12`, `13` and `14` are **GPIO pins**,
`SR8Pin::...` are **chip tags**,
`1`-`8` are **input numbers**, and
`55` is the input number assigned to the `SER` tag in `chip2`.

## Simwheel functions (`inputHub` namespace)

API calls have been **renamed** according to this table (same parameters):

| Before                                               | Now                                                       |
| ---------------------------------------------------- | --------------------------------------------------------- |
| inputHub::setClutchInputNumbers()                    | inputHub::clutch::inputs()                                |
| inputHub::setClutchCalibrationInputNumbers()         | inputHub::clutch::bitePointInputs()                       |
| inputHub::cycleCPWorkingMode_setInputNumbers         | inputHub::clutch::cycleWorkingModeInputs()                |
| inputHub::cmdRecalibrateAnalogAxis_setInputNumbers() | inputHub::clutch::cmdRecalibrateAxisInputs()              |
| inputHub::setDPADControls()                          | inputHub::dpad::inputs()                                  |
| inputHub::cycleDPADWorkingMode_setInputNumbers       | inputHub::dpad::cycleWorkingModeInputs()                  |
| inputHub::setALTInputNumbers()                       | inputHub::altButtons::inputs()                            |
| inputHub::cycleALTButtonsWorkingMode_setInputNumbers | inputHub::altButtons::cycleWorkingModeInputs()            |
| inputHub::cycleSecurityLock_setInputNumbers()        | inputHub::securityLock::cycleWorkingModeInputs()          |
| inputHub::cpWorkingMode_setInputNumbers()            | (**removed, use the companion app or the cycle command**) |
| inputHub::cmdRecalibrateBattery_setInputNumbers      | (**removed, use the companion app**)                      |

## HID

The `hidImplementation` namespace has been renamed to `hid`.
`hidImplementation::begin()` has been renamed to `hid::configure()`.
The parameters are slightly different, as you can now specify a "factory"
*vendor ID* now (a "factory" *product ID* was already available before).
The "factory" hardware ID is ignored by the USB implementation (as before).

## User interface

The `notify` namespace has been renamed to `ui`.
`notify::begin()` has been removed.

- The `AbstractUserInterface` class has `getMaxFPS()` and
  `getStackSize()` members that replace some parameters
  of `notify::begin()`.
  You will need to override these if you have custom code.
- Now you call `ui::add()` as many times as needed,
  instead of providing a group of instances to `notify::begin()`.
- `ui::addPixelControlNotifications()` does the work for pixel
  control notifications (just a shortcut).

Now, each `AbstractUserInterface` instance runs on its own low-priority thread.
This **takes more memory**, but:

- Notifications are truly concurrent.
- Each instance will run at the required FPS independently of the others.
- Each instance can have a different stack size.

## Running

`inputs::start()` is no longer necessary.
You make a single call to `firmware::run()` to run your custom firmware.
This method will catch firmware errors for display via the serial port.

## Battery monitor subsystem (`batteryMonitor` namespace)

`batteryMonitor::begin()` has been replaced by `batteryMonitor::configure()`.
Beware that `batteryMonitor::configure()` takes parameters
**in reverse order** compared to the old `batteryMonitor::begin()` method
when customizing the simple voltage divider or the battery monitor.
[CustomSetup.ino](../src/Firmware/CustomSetup/CustomSetup.ino) already
takes this into account.
When customizing the "fuel gauge", parameters are slightly different too:

1. I2C bus where the fuel gauge is attached to.
   Previously, only the primary bus was available.
   By default, the primary bus is choosen.
2. I2C address (as before).

## Power subsystem (`power` namespace)

`power::begin()` has been replaced by `power::configureWakeUp()` (same parameters).
`power::setPowerLatch()` has been replaced by `power::configurePowerLatch()` (same paramters).

## New (optional) features

### Default input map

You can now specify a default input map for your input numbers.
This way, you are not forced to use the full range of 128 gamepad buttons
if you only have a few physical buttons.
This feature does not interfere with the ability to set a different input map
using the *companion app*.

Just call `inputMap::set()`.
The parameters are (from left to right):

- A firmware-defined input number (must be assigned to the input hardware).
- A user-defined input number (in the range [0,127])
  to be used when ALT mode is not engaged.
- A user-defined input number (in the range [0,127])
  to be used when ALT mode is engaged.

Make as many calls as you wish.

### "Virtual" button

This is a firmware-defined input number that is not assigned to any input hardware.
Instead, you provide a combination of (existing) input numbers that will be
replaced by the "virtual" input number when pressed simultaneously.
This is intended for a "neutral gear" button when both shift paddles are engaged,
but you can use it in any way you like.

Call `inputHub::neutralGear::inputs()` to assign
a "virtual" input number and combination of input numbers.
