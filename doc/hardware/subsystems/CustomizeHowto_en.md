# How to customize in order to build your own hardware and firmware setup

The whole sim wheel/button box system has been divided into pieces called **subsystems**. Each subsystem performs a particular function and is composed both by hardware and (sometimes) software. A description of each subsystem is available below.
In order to build your own setup, follow a few simple steps:

1. Choose what is required from each subsystem
2. Design your custom hardware
3. Configure your custom firmware
4. *Optional but recommended*: build the resulting circuit in a protoboard, upload your custom firmware and test it.
5. Build your design into a [perfboard](https://en.wikipedia.org/wiki/Perfboard).

## Choose what is required from each subsystem

Follow the links to get a detailed description of each subsystem:

- [Power subsystem](./Power/Power_en.md): provides a power source to the system.
- [Power latch subsystem](./PowerLatch/PowerLatch_en.md): provides power on/off functionality.
- [Battery monitor subsystem](./BatteryMonitor/BatteryMonitor_en.md): provides an estimation of battery charge.
- [Relative rotary encoder subsystem](./RelativeRotaryEncoder/RelativeRotaryEncoder_en.md): provides input from rotation of rotary encoders.
- [Switches subsystem](./Switches/Switches_en.md): provides input from normally-open momentary switches (push buttons, paddles, etc.) and potentiometers (as on/off inputs, only).
- [Analog clutch subsystem](./AnalogClutchPaddles/AnalogClutchPaddles_en.md): provides input from two potentiometers attached to clutch paddles as analog axes.

### About clutch paddles

This project support two kinds of clutch paddles:

- **"Analog" clutch paddles**.
  Take input from potentiometers exclusively, as described in the *analog clutch subsystem*.
  This is the best choice. Two ADC-capable pins are required.

- **"Digital" clutch paddles**.
  Take input from the *switches subsystem*, including any push button or roller lever (not just "paddles").
  If no ADC-capable pins are available, you may still use potentiometers as "digital" clutch paddles by attaching them
  to the *switches subsystems* instead of the *analog clutch subsystem*.

## Design your custom hardware

Depending on how much space is available in the wheel's case, you have the choice to build all the subsystems in a single perfboard, or split them into several perfboards at the cost of extra wiring.

1. Open [DIY layout creator](https://bancika.github.io/diy-layout-creator/) and create a new project file.
2. Open the provided `.diy` file for each subsystem. Copy-paste whatever you need to your own project file. Use the "edit" menu since keyboard shortcuts does not seem to work. You may also start from a predefined setup.
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

3. Map certain input numbers to specific functions, as explained below. Edit the body of `simWheelSetup()` and place the required calls at the end of it.
   All of those mappings are optional, but take care not to build a dis functional firmware.
   Do not assign two functions to the same input numbers.
   Where available, do not use a combination of input numbers which can not be activated at the same time.
   Do not map a specific function to non-existent input numbers.
   If you choose not to map those specific functions, they are still available thanks to the companion app.

*Note:* "..." means other code not explicitly shown.

### DPAD

A DPAD is optional. Despite this function is designed for funky switches and directional pads, it may be assigned to *any* input, including rotary encoders and push buttons. Place a call to `inputHub::setDPADControls()`:

- 1st parameter is the input number for the "up" button
- 2nd parameter is the input number for the "down" button
- 3rd parameter is the input number for the "left" button
- 4th parameter is the input number for the "right" button

For example, let's say the button matrix contains input numbers 20, 22, 25 and 28:

```c
void simWheelSetup()
{
   ...
   inputs::addButtonMatrix(...)
   ...
   inputHub::setDPADControls(20, 22, 25, 28);
   ...
}
```

### Cycle working mode of DPAD

Each time this function is activated, the working mode of the DPAD will move to the next one : navigation controls, regular buttons and back to the first mode.
There is no point on this if there is no DPAD.

Assign a combination of input numbers to activate this function by placing a call to
`inputHub::cycleDPADWorkingMode_setInputNumbers()`. There is one parameter: a sequence of input numbers between brackets.
All the inputs have to be active at the same time, and none of the others.

For example:

```c
void simWheelSetup()
{
   ...
   inputHub::cycleDPADWorkingMode_setInputNumbers({60,61});
   ...
}
```

### Clutch paddles

Clutch paddles are optional. You can have *analog* or *digital* clutch paddles. They work just the same.

Place a call to `inputHub::setClutchInputNumbers()`. Each parameter (there are two) is the input number assigned to a clutch paddle.
For example, let's say the button matrix contains input numbers 45 and 46:

```c
void simWheelSetup()
{
   ...
   inputs::addButtonMatrix(...)
      .inputNumber(...,45)
      .inputNumber(...,46)
      ...
   ...
   inputHub::setClutchInputNumbers(45, 46);
   ...
}
```

Now, those input numbers may work as clutch paddles depending on user preferences.

It works the same for analog clutch paddles, but no existing input number is required, as shown in the following example:

```c
void simWheelSetup()
{
   inputs::setAnalogClutchPaddles(...);
   ...
   inputHub::setClutchInputNumbers(45, 46);
   ...
}
```

Now, the analog clutch paddles may work as input numbers 45 and 46 depending on user preferences.

### Clutch's bite point calibration

Place a call to `inputHub::setClutchCalibrationInputNumbers()`:

- 1st parameter is an input number to increase the bite point.
- 2nd parameter is an input number to decrease the bite point.

For example, let's say this function is mapped to a rotary encoder (input numbers 34 and 35):

```c
void simWheelSetup()
{
   inputs::addRotaryEncoder(...,34,35);
   ...
   inputHub::setClutchCalibrationInputNumbers(34, 35);
   ...
}
```

There is no point on this if there are no clutch paddles.

### ALT buttons

You may assign this function to any number of buttons (or none). Place a call to `inputHub::setALTInputNumbers()`.
There is one parameter: a sequence of input numbers between brackets.

For example, let's say this function is mapped to two certain inputs at the button matrix:

```c
void simWheelSetup()
{
   ...
   inputs::addButtonMatrix(...)
      .inputNumber(...,45)
      .inputNumber(...,46)
      ...
   ...
   inputHub::setALTInputNumbers({45,46});
   ...
}
```

Any of the given input numbers will engage "ALT" mode when activated, except if they are set to work as "regular buttons" by the user.

### Cycle working mode for clutch paddles

Each time this function is activated, the working mode of the clutch paddles will move to the next one :
F1-style clutch,
autonomous axes,
"ALT" mode,
regular buttons,
launch control (left paddle is master),
launch control (right paddle is master)
and back to the first mode.
There is no point on this if there are no clutch paddles.

Assign a combination of input numbers to activate this function by placing a call to
`inputHub::cycleCPWorkingMode_setInputNumbers()`. There is one parameter: a sequence of input numbers between brackets.
All the inputs have to be active at the same time, and none of the others.
For example:

```c
void simWheelSetup()
{
   ...
   inputs::addButtonMatrix(...)
      .inputNumber(...,60)
      .inputNumber(...,61)
      .inputNumber(...,62)
      ...
   ...
   inputHub::cycleCPWorkingMode_setInputNumbers({60,61,62});
   ...
}
```

#### Select a specific working mode for clutch paddles

As an alternative, you may assign specific button combinations to specific working modes.
Place a call to `inputHub::cpWorkingMode_setInputNumbers()`.
There are four parameters.
Each one must contain a sequence of input numbers between brackets as seen in the previous calls.
From left to right:

1. Button combination to select F1-Style clutch mode.
2. Button combination to select autonomous axes mode.
3. Button combination to select "ALT" mode.
4. Button combination to select "regular buttons" mode.
5. Optional: button combination to select "launch control" mode (left paddle is master).
6. Optional: button combination to select "launch control" mode (right paddle is master).

For example:

```c
void simWheelSetup()
{
   ...
   inputs::addButtonMatrix(...)
      .inputNumber(...,59)
      .inputNumber(...,60)
      .inputNumber(...,61)
      .inputNumber(...,62)
      .inputNumber(...,63)
      ...
   ...
   inputHub::cpWorkingMode_setInputNumbers({59,60}, {59,61}, {59,62}, {59,63});
   ...
}
```

### Cycle working mode of "ALT" buttons

Each time this function is activated, the working mode of the "ALT" buttons will move to the next one : "ALT" mode, regular buttons and back to the first mode.
There is no point on this if there are no "ALT" buttons.

Assign a combination of input numbers to activate this function by placing a call to
`inputHub::cycleALTButtonsWorkingMode_setInputNumbers()`.
There is one parameter: a sequence of input numbers between brackets.
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

This project provides three (exclusive) connectivity choices:

- Bluetooth Low Energy (BLE) using the [NimBLE stack](https://mynewt.apache.org/latest/network/). This is the default. If you are happy with this, ignore this section.

- Bluetooth Low Energy using the ESP32-Arduino stack. Currently not used (except for a test unit), but available.
  This stack requires more flash memory compared to *NimBLE* (about 500 KB).
  If you have issues with *NimBLE*, this is a workaround.

- Universal Serial Bus (USB).

In order to use any of them:

- Edit the file **"includes.txt"** at your sketch folder (under [src/Firmware](../../../src/Firmware/)). Replace the text "hidImplementation_NimBLE.cpp" with a filename chosen from this table:

  | Connectivity | Stack         | Filename                     |
  | ------------ | ------------- | ---------------------------- |
  | BLE          | NimBLE        | hidImplementation_NimBLE.cpp |
  | BLE          | ESP32-Arduino | hidImplementation_ESPBLE.cpp |
  | USB          | ESP32-Arduino | hidImplementation_USB.cpp    |

- Do not confuse those with "hidImplementation_common.cpp". Do not touch that line.
- Run the [sources setup procedure](../../firmware/sourcesSetup_en.md) again. **This is mandatory**.

If you go for a purely wired USB implementation:

- Set USB-Mode to "USB-OTG (TinyUSB)" in Arduino-IDE (board configuration).
- There is no sense in using a battery-operated design if you have USB power available. For this reason, automatic power-off is not available within this implementation.
- Note that you can not have both Bluetooth and USB at the same time in the same device.

### Security lock

For security concerns, the user can lock or unlock configuration changes coming from any PC application (including the companion app).
This is a security precaution to stop unauthorized configuration modifications caused by rogue programs.

Assign a combination of input numbers to activate this function by placing a call to
`inputHub::cycleSecurityLock_setInputNumbers()`.
There is one parameter: a sequence of input numbers between brackets.
All the inputs have to be active at the same time, and none of the others.
For example:

```c
void simWheelSetup()
{
   ...
   inputs::addButtonMatrix(...)
      .inputNumber(...,50)
      .inputNumber(...,51)
      ...
   ...
   inputHub::cycleSecurityLock_setInputNumbers({50,51});
   ...
}
```

The state of the security lock is changed on each activation,
then saved to flash memory after a short delay.

## Build your design into a perfboard

Some of the circuit designs may show very small resistors and diodes which does not fit the real ones. This is not a mistake. They must be placed in "vertical" layout, so they lie in a minimal surface of the perfboard.

![Vertical layout](../pictures/VerticalLayout.png)

Note that some components may be placed on top of others to save more space.
