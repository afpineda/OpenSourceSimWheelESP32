# How to customize in order to build your own hardware and firmware setup

The whole sim wheel/button box system has been divided into pieces called **subsystems**.
Each subsystem performs a particular function and
is composed both by hardware and (sometimes) software.
A description of each subsystem is available below.
In order to build your own setup, follow a few simple steps:

1. Choose what is required from each subsystem
2. Design your custom hardware
3. Configure your custom firmware
4. *Optional but recommended*: build the resulting circuit in a protoboard,
   upload your custom firmware and test it.
5. Build your design into a [perfboard](https://en.wikipedia.org/wiki/Perfboard).

## Choose what is required from each subsystem

Follow the links to get a detailed description of each subsystem:

- [Power subsystem](./Power/Power_en.md):
  provides a power source to the system.
- [Power latch subsystem](./PowerLatch/PowerLatch_en.md):
  provides power on/off functionality.
- [Battery monitor subsystem](./BatteryMonitor/BatteryMonitor_en.md):
  provides an estimation of battery charge.
- [Relative rotary encoder subsystem](./RelativeRotaryEncoder/RelativeRotaryEncoder_en.md):
  provides input from rotation of rotary encoders.
- [Switches subsystem](./Switches/Switches_en.md):
  provides input from normally-open momentary switches (push buttons, paddles, etc.)
  and potentiometers (as on/off inputs, only).
- [Analog clutch subsystem](./AnalogClutchPaddles/AnalogClutchPaddles_en.md):
  provides input from two potentiometers attached to clutch paddles as analog axes.

### About clutch paddles

This project support two kinds of clutch paddles:

- **"Analog" clutch paddles**.
  Take input from potentiometers exclusively, as described in the *analog clutch subsystem*.
  This is the best choice. Two ADC-capable pins are required.

- **"Digital" clutch paddles**.
  Take input from the *switches subsystem*, including any push button
  or roller lever (not just "paddles").
  If no ADC-capable pins are available, you may still use potentiometers
  as "digital" clutch paddles by attaching them
  to the *switches subsystems* instead of the *analog clutch subsystem*.

## Design your custom hardware

Depending on how much space is available in the wheel's case,
you have the choice to build all the subsystems in a single perfboard,
or split them into several perfboards at the cost of extra wiring.

1. Open [DIY layout creator](https://bancika.github.io/diy-layout-creator/) and create a new project file.
2. Open the provided `.diy` file for each subsystem.
   Copy-paste whatever you need to your own project file.
   Use the "edit" menu since keyboard shortcuts does not seem to work.
   You may also start from a predefined setup.
3. Re-arrange the components to fit your needs.

## Configure your custom firmware

Arduino's sketch named
[**CustomSetup**](../../../src/Firmware/CustomSetup/CustomSetup.ino)
is the place where you build your own firmware by modifying a few specific lines of code.
You may want to make a copy first to prevent your customized code from being deleted after updating.
To make a working copy:

- Copy the entire folder under a different name.
- Make sure that both the folder and the ".ino" file inside it have exactly the same name.

Then, there are a few steps:

1. Configure each subsystem as described in their documentation.

2. Configure inputs:

   As shown for each subsystem, a call to a function in the `inputs` namespace will enable them.
   You must also assign a unique "input number" to each input, **in the range from 0 to 63**.
   If you fail to provide valid input numbers, the firmware will not boot up.
   Each input number corresponds to a certain position in a pin header in your hardware design.
   Some input numbers have a certain meaning in the hosting PC.

3. Map certain input numbers to specific functions using the `inputHub` namespace, as explained below.
   Edit the body of `simWheelSetup()` and place the required calls at the end of it.
   All of those mappings are optional, but take care not to build a non-functional firmware.
   Do not assign two functions to the same input numbers.
   Where available, do not use a combination of input numbers which can not be activated at the same time.
   Do not map a specific function to non-existent input numbers.
   If you choose not to map those specific functions, they are still available thanks to the companion app.

*Note:* "..." means other code not explicitly shown.

### DPAD

A DPAD is optional.
Despite this function is designed for funky switches and directional pads,
it may be assigned to *any* input,
including rotary encoders and push buttons.
Place a call to `inputHub::dpad::inputs()`:

- 1st parameter is the input number for the "up" button
- 2nd parameter is the input number for the "down" button
- 3rd parameter is the input number for the "left" button
- 4th parameter is the input number for the "right" button

For example, let's say the button matrix contains
the input numbers 20, 22, 25 and 28:

```c
void simWheelSetup()
{
   ...
   inputHub::dpad::inputs(20, 22, 25, 28);
   ...
}
```

### Cycle the working mode of the DPAD

Each time this function is activated,
the working mode of the DPAD will move to the next one:
navigation controls, regular buttons and back to the first mode.
There is no point on this if there is no DPAD.

Assign a combination of input numbers to activate this function by placing a call to
`inputHub::dpad::cycleWorkingModeInputs()`.
There is one parameter: a sequence of input numbers between braces.
All the inputs have to be active at the same time, and none of the others.

For example:

```c
void simWheelSetup()
{
   ...
   inputHub::dpad::cycleWorkingModeInputs({60,61});
   ...
}
```

### Clutch paddles

Clutch paddles are optional.
You can have *analog* or *digital* clutch paddles.
They work just the same.

Place a call to `inputHub::clutch::inputs()`. Each
 parameter (there are two) is the input number assigned to a clutch paddle.
For example, let's say the button matrix contains input numbers 45 and 46:

```c
void simWheelSetup()
{
   ...
   inputHub::clutch::inputs(45, 46);
   ...
}
```

Now, those input numbers may work as clutch paddles depending on user preferences.

It works the same for analog clutch paddles,
but no existing input number is required,
as shown in the following example:

```c
void simWheelSetup()
{
   inputs::setAnalogClutchPaddles(...);
   ...
   inputHub::clutch::inputs(45, 46);
   ...
}
```

Now, the analog clutch paddles may work as input numbers 45 and 46 depending on user preferences.

### Bite point calibration

This feature is optional.
Bite point calibration is also available in the companion app.

Place a call to `inputHub::clutch::bitePointInputs()`:

- 1st parameter is an input number to increase the bite point.
- 2nd parameter is an input number to decrease the bite point.

For example (input numbers 34 and 35):

```c
void simWheelSetup()
{
   ...
   inputHub::clutch::bitePointInputs(34, 35);
   ...
}
```

There is no point on this if there are no clutch paddles.

### Alternate mode

You may assign this function to any number of buttons (or none).
Place a call to `inputHub::altButtons::inputs()`.
There is one parameter: a sequence of input numbers between braces.

For example:

```c
void simWheelSetup()
{
   ...
   inputHub::altButtons::inputs({45,46});
   ...
}
```

Any of the given input numbers will engage *alternate mode* when activated,
except if they are set to work as "regular buttons" by the user.

### Cycle the working mode of the clutch paddles

Each time this function is activated,
the working mode of the clutch paddles will move to the next one :
F1-style clutch,
autonomous axes,
alternate mode,
regular buttons,
launch control (left paddle is master),
launch control (right paddle is master)
and back to the first mode.
There is no point on this if there are no clutch paddles.

Assign a combination of input numbers to activate this function by placing a call to
`inputHub::clutch::cycleWorkingModeInputs()`.
There is one parameter: a sequence of input numbers between braces.
All the inputs have to be active at the same time, and none of the others.
For example:

```c
void simWheelSetup()
{
   ...
   inputHub::clutch::cycleWorkingModeInputs({60,61,62});
   ...
}
```

### Cycle the working mode of the "ALT" buttons

Each time this function is activated,
the working mode of the "ALT" buttons will move to the next one:
alternate mode, regular buttons and back to the first mode.
There is no point on this if there are no "ALT" buttons.

Assign a combination of input numbers to activate this function by placing a call to
`inputHub::altButtons::cycleWorkingModeInputs()`.
There is one parameter: a sequence of input numbers between braces.
All the inputs have to be active at the same time, and none of the others.

### Other game pad controls

Note that the following input numbers have a special meaning in Windows:

- *00*: "A" button
- *01*: "B" button
- *02*: "X" button
- *03*: "Y" button
- *04*: "LB" button (should be reserved for the left shift paddle)
- *05*: "RB" button (should be reserved for the right shift paddle)
- *06*: "Back" button
- *07*: "Start" button

### Connectivity

This project provides several (exclusive) connectivity choices:

- Bluetooth Low Energy (BLE) using the
  [NimBLE stack](https://mynewt.apache.org/latest/network/) wrapper
  from [h2zero](https://github.com/h2zero).
  This is the default.
  If you are happy with this, ignore this section.

- Bluetooth Low Energy using the NimBLE stack wrapper from the ESP32-Arduino core.
  Currently not used (except for a test unit), but available.
  This stack requires more flash memory compared to *h2zero/NimBLE-Arduino* (about 500 KB).
  If you have issues with *NimBLE*, this is a workaround.

- Universal Serial Bus (USB).

- "Dummy" connectivity, available for troubleshooting.
  This option provides no connectivity at all.
  Switch to this temporarily if your firmware won't boot and
  you don't have access to the serial port.
  This is the case when using USB connectivity on a DevKit board
  with only one USB header.

In order to use any of them:

- Edit the file **"includes.txt"** at your sketch folder.
  Replace the text "hid_NimBLE.cpp" with a filename chosen from this table:

  | Connectivity | Stack         | Filename       |
  | ------------ | ------------- | -------------- |
  | BLE          | NimBLE        | hid_NimBLE.cpp |
  | BLE          | ESP32-Arduino | hid_ESPBLE.cpp |
  | USB          | ESP32-Arduino | hid_USB.cpp    |
  | Dummy        | none          | hid_dummy.cpp  |

  Those file names are case-sensitive if you have Linux or Mac.

- Do not confuse those with "hidCommon.cpp".
  Do not touch that line.
- Run the [sources setup procedure](../../firmware/sourcesSetup_en.md) again.
  **This is mandatory**.

If you go for a purely wired USB implementation:

- Set USB-Mode to "USB-OTG (TinyUSB)" in Arduino-IDE (board configuration).
- There is no sense in using a battery-operated design if you have USB power available.
  For this reason, automatic shutdown is not available within this implementation.
- Note that you can not have both Bluetooth and USB at the same time in the same device.

### Security lock

For security concerns,
the user can lock or unlock configuration changes coming from any PC application
(including the companion app).
This is a security precaution to stop unauthorized configuration modifications caused by rogue programs.

Assign a combination of input numbers to activate this function by placing a call to
`inputHub::securityLock::cycleWorkingModeInputs()`.
There is one parameter: a sequence of input numbers between braces.
All the inputs have to be active at the same time, and none of the others.
For example:

```c
void simWheelSetup()
{
   ...
   inputHub::securityLock::cycleWorkingModeInputs({50,51});
   ...
}
```

The state of the security lock is changed on each activation,
then saved to flash memory after a short delay.

### Default input map

This feature is optional.

Regardless of the input numbers assigned to each input hardware,
the end user can set up a custom input map using the companion application.
Unlike firmware defined input numbers,
user defined input numbers are in the range [0,127].
By default, the input map follows this rule:

- If *alternate mode* is not engaged,
  the user defined input number is the firmware defined input number.
- If *alternate mode* is engaged,
  the user defined input number is the firmware defined input number plus 64.

If you have only a few buttons,
their user-defined input numbers will be spread over 128 values.
However, you can set a **different default input map**.
To the end user, your custom input map is considered the "factory default".
There are several, **non-exclusive**, options.

#### inputMap::setOptimal()

Make a single call to `inputMap::setOptimal()` (no arguments).
The input map will follow this rule:

- If *alternate mode* is not engaged,
  the user defined input number is the firmware defined input number.
- If *alternate mode* is engaged,
  the user defined input number is the firmware defined input number,
  plus the highest firmware defined input number, plus one.

For example, let's say you have the input numbers `1`, `3` and `5`
(and no others) assigned to your input hardware.
This method will set the following input map:

| firmware-defined | user-defined | user-defined (alternate mode) |
| :--------------: | :----------: | :---------------------------: |
|        1         |      1       |               7               |
|        3         |      3       |               9               |
|        5         |      5       |              11               |

#### inputMap::set() with three arguments

Make calls to `inputMap::set()` with the following parameters (from left to right):

1. A firmware-defined input number: must be assigned to the input hardware.
2. A user-defined input number to be reported when *alternate mode* is **not** engaged.
3. A user-defined input number to be reported when *alternate mode* **is** engaged.

Make as many calls as you need.
For example:

```c++
inputs::addButton(GPIO_NUM_20, 1);
inputs::addButton(GPIO_NUM_21, 2);
inputs::addButton(GPIO_NUM_22, 3);
inputHub::altButtons::inputs({3});
inputMap::set(1, 0, 2); // Line 1
inputMap::set(2, 1, 3); // Line 2
inputMap::set(3, 4, 4); // Line 3
```

When:

- The user pushes the button attached to GPIO 20:
  - The firmware defined input number 1 is detected as pushed.
  - But the user defined input number 0 is reported to the hosting PC
    as pushed, due to "Line 1", second parameter.
- The user pushes the buttons attached to GPIO 21 and GPIO 22, at the same time:
  - The firmware defined input numbers 2 and 3 are detected as pushed
  - Since the input number 3 is the ALT button,
    *alternate mode* is engaged and the input number 3 is ignored.
  - The user defined input number 3 is reported to the hosting PC
    as pushed, due to "Line 2", **third** parameter.
- ALT buttons are configured as regular buttons
  (using the companion app) and the button attached to GPIO 22 is pushed:
  - The firmware defined input number 3 is detected as pushed.
  - But the user defined input number 4 is reported to the hosting PC
    as pushed, due to "Line 3", second parameter.
    Note that the third parameter is useless for "ALT" buttons.

#### inputMap::set() with two arguments

Make calls to `inputMap::set()` with the following parameters (from left to right):

1. A firmware-defined input number: must be assigned to the input hardware.
   This is also the user-defined input number to be reported
   when *alternate mode* **is not** engaged.
2. A user-defined input number to be reported when *alternate mode* **is** engaged.

This is a shortcut. Make as many calls as you need.

### Neutral gear ("virtual" button)

This feature is optional.

The *neutral gear* is an input number that is not assigned to any input hardware
(but this is not mandatory).
When the end-user presses a particular combination of buttons,
the *neutral gear* is reported instead.
The end-user must then fully release that combination for those
buttons to behave normally.

Make a call to `inputHub::neutralGear::set()`
with the following parameters (from left to right):

1. The desired *neutral gear* input number.
2. A combination of input numbers between braces
   that engages the neutral gear.

This function is intended for shift paddles,
but you can assign this function to any combination of input numbers.
"Neutral gear" is the intended use for this "virtual" button,
but it is not a firmware function.
You must assign the virtual button to *neutral*
or any other function in-game.

Note that the neutral gear will be engaged if the button
combination is pressed *and* any of the others.

The "virtual" input number is affected by the *alternate mode* and
can be mapped to other user-defined input numbers.

Example:

```c++
inputs::addButton(GPIO_NUM_20, 1); // Left shift paddle
inputs::addButton(GPIO_NUM_21, 2); // Right shift paddle
inputHub::neutralGear::set(3,{1,2}); // Neutral gear
```

Let's suppose this sequence of events:

- The left shift paddle is pushed:
  - Button 1 is reported as pressed (to the hosting PC)
  - Button 2 is reported as released
  - Button 3 is reported as released
- The right shift paddle is pushed (both are pushed now):
  - Button 1 is reported as released (to the hosting PC)
  - Button 2 is reported as released
  - Button 3 is reported as **pushed**
- The right shift paddle is released:
  - Button 1 is reported as **released** (to the hosting PC)
  - Button 2 is reported as **released**
  - Button 3 is reported as released
- The left shift paddle is released:
  - Button 1 is reported as **released** (to the hosting PC)
  - Button 2 is reported as **released**
  - Button 3 is reported as released

Once both shift padles are released,
they will be reported as buttons 1 and 2 again.

## Build your design into a perfboard

Some of the circuit designs may show very small resistors and diodes which does not fit the real ones.
This is not a mistake.
They must be placed in "vertical" layout,
so they lie in a minimal surface of the perfboard.

![Vertical layout](../pictures/VerticalLayout.png)

Note that some components may be placed on top of others to save more space.
