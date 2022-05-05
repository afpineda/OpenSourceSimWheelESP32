# How to customize in order to build your own hardware and firmware setup

The whole sim wheel/button box system has been divided into pieces called **subsystems**. Each subsystem performs a particular function and is composed both by hardware and (sometimes) software. A description of each subsystem is available below.
In order to build your own setup, follow a few simple steps:

1. Choose what is required from each subsystem
2. Design your custom hardware
3. Configure your custom firmware
4. _Optional but recommended_: build the resulting circuit in a protoboard, upload your custom firmware and test it.
5. Build your design into a [perfboard](https://en.wikipedia.org/wiki/Perfboard).

## Choose what is required from each subsystem

Follow the links to get a detailed description of each subsystem:

- [Power subsystem](./Power/Power_en.md): provides a power source to the system.
- [Power latch subsystem](./PowerLatch/PowerLatch_en.md): provides power on/off functionality.
- [Battery monitor subsystem](./BatteryMonitor/BatteryMonitor_en.md): provides an estimation of battery charge.
- [Relative rotary encoder subsystem](./RelativeRotaryEncoder/RelativeRotaryEncoder_en.md): provides input from rotation of rotary encoders.
- [Switches subsystem](./Switches/Switches_en.md): provides input from NO momentary switches (push buttons, paddles, etc.) and potentiometers (some kinds of clutch paddles).
- [Display subsystem](./Display/Display_en.md): provides a user interface.

## Design your custom hardware

Depending on how much space is available in the wheel's case, you have the choice to build all the subsystems in a single perfboard, or split them into several perfboards at the cost of extra wiring.

1. Open [DIY layout creator](https://bancika.github.io/diy-layout-creator/) and create a new project file.
2. Open the provided `.diy` file for each subsystem. Copy-paste whatever you need to your own project file. Use the "edit" menu since keyboard shortcuts does not seem to work. You may also start from a predefined setup.
3. Re-arrange the components to fit your needs.

## Configure your custom firmware

Arduino's sketch named [**CustomSetup**](../../../src/Firmware/CustomSetup/CustomSetup.ino) is the place where you build your own firmware by modifying a few specific lines of code. There are few steps:

1. Configure each subsystem as described in their documentation.

2. Configure inputs:
   
   As shown for each subsystem, a call to a function in the `inputs` namespace will enable them. Those functions will return the assigned number for the first input, but you can figure out in advance. The assigned number to certain inputs must be known for the next step. Each input number also maps to a certain position in a pin header at your hardware design.

3. Map certain input numbers to specific functions, as explained below. Edit the body of `simWheelSetup()` and place the required calls at the end of it. All of those mappings are optional, but take care not to build a dis functional firmware. Do not assign two functions to the same input number (except for menu navigation).

*Note:* "..." means other code not explicitly shown.

### DPAD

Despite this function is designed for funky switches and directional pads, it may be assigned to _any_ input, including rotary encoders and push buttons. Place a call to `inputHub::setDPADControls()`:

- 1st parameter is the input number for the "up" button
- 2nd parameter is the input number for the "down" button
- 3rd parameter is the input number for the "left" button
- 4th parameter is the input number for the "right" button

For example, let's say this function is mapped to certain inputs at the button matrix:

```c
void simWheelSetup()
{
   ...
   inputNumber_t mtxFirstBtn = inputs::setButtonMatrix(...);
   ...
   inputHub::setDPADControls(
      mtxFirstBtn+10,
      mtxFirstBtn+12,
      mtxFirstBtn+16,
      mtxFirstBtn+18);
   ...
}
```

### Clutch paddless

Place a call to `inputHub::setClutchPaddles()`. Each parameter (there are two) is the input number assigned to a clutch paddle.
For example, let's say this function is mapped to certain inputs at the button matrix:

```c
void simWheelSetup()
{
   ...
   inputNumber_t mtxFirstBtn = inputs::setButtonMatrix(...);
   ...
   inputHub::setClutchPaddles(
      mtxFirstBtn+14,
      mtxFirstBtn+15);
   ...
}
```

### Clutch's bite point calibration

Place a call to `inputHub::setClutchCalibrationButtons()`:

- 1st parameter is an input number to increase the bite point.
- 2nd parameter is an input number to decrease the bite point.

For example, let's say this function is mapped to a rotary encoder:

```c
void simWheelSetup()
{
   ...
   inputNumber_t rotary1Clockwise = inputs::addRotaryEncoder(...);
   ...
   inputHub::setClutchCalibrationButtons(
      rotary1Clockwise,
      rotary1Clockwise+1);
   ...
}
```

### ALT buttons

You may assign this function to any number of buttons (or none). Place a call to `inputHub::setALTBitmap()`.
There is one parameter: a sequence of calls to `BITMAP(<input number>)` separated by `|`.

For example, let's say this function is mapped to two certain inputs at the button matrix:

```c
void simWheelSetup()
{
   ...
   inputNumber_t mtxFirstBtn = inputs::setButtonMatrix(...);
   ...
   inputHub::setALTBitmap(
      BITMAP(mtxFirstBtn) | BITMAP(mtxFirstBtn+1)
   );
   ...
}
```

### Enter/exit menu

This function is useless if there is no OLED subsystem. You may assign this function to any number of push buttons (rotation of a rotary encoder will not work). All of those buttons, and none of the others, have to be pressed at the same time, hold for two seconds, then released, to enter/exit the configuration menu. If those buttons are pressed in any other way, **they will be reported to the hosting PC** as any other input. To avoid accidental activation, assign two button numbers.

Place a call to `inputHub::setMenuBitmap()`. There is one parameter: a sequence of calls to `BITMAP(<input number>)` separated by `|`. For example, let's say this function is mapped to two certain buttons at the button matrix:

```c
void simWheelSetup()
{
   ...
   inputNumber_t mtxFirstBtn = inputs::setButtonMatrix(...);
   ...
   inputHub::setMenuBitmap(
      BITMAP(mtxFirstBtn+20) | BITMAP(mtxFirstBtn+22)
   );
   ...
}
```

### Menu navigation

This function is useless if there is no OLED subsystem. 

In this case, assigned input numbers can be shared with other functions. Place a call to `configMenu::setNavButtons()`:

- 1st parameter is the input number assigned to navigate to the previous option.
- 2nd parameter is the input number assigned to navigate to the next option.
- 3rd parameter is the input number assigned to select a menu option.
- 4th parameter is the input number assigned to cancel.

For example, let's say navigation is mapped to a rotary encoder (the same as a previous example), "select" is mapped to its built in push button and "cancel" is mapped to a clutch paddle (the same as a previous example):

```c
void simWheelSetup()
{
   ...
   inputNumber_t mtxFirstBtn = inputs::setButtonMatrix(...);
   inputNumber_t rotary1Clockwise = inputs::addRotaryEncoder(...);
   inputNumber_t rotary1Button = inputs::addDigital(...);
   ...
   inputHub::setClutchPaddles(
      mtxFirstBtn+14,
      mtxFirstBtn+15);
   inputHub::setClutchCalibrationButtons(
      rotary1Clockwise,
      rotary1Clockwise+1);
   ...
   configMenu::setNavButtons(
      rotary1Clockwise,
      rotary1Clockwise+1,
      rotary1Button,
      mtxFirstBtn+14
   );
   ...
}
```

### Other game pad controls

Note that the following input numbers have a special meaning in Windows:

- _00_: "A"
- _01_: "B"
- _02_: "X"
- _03_: "Y"
- _04_: "LB" (should be reserved for the left shift paddle)
- _05_: "RB" (should be reserved for the right shift paddle)
- _06_: "Back"
- _07_: "Start"

## Build your design into a perfboard

Some of the circuit designs may show very small resistors and diodes which does not fit the real ones. This is not a mistake. They must be placed in "vertical" layout, so they lie in a minimal surface of the perfboard.

<img src="../pictures/VerticalLayout.png" alt="Vertical layout" width="40%" margin-lefet="auto" />

Note that some components may be placed on top of others to save more space.
