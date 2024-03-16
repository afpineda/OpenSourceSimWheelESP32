# Migrating your custom firmware to version 4 from a previous version

Starting with version 4, there are some important improvements to the `inputs` and `inputHub` namespaces.
Unfortunately, any custom firmware needs a rewrite.

For more details, please, read again the following documentation:

- [Customization guide](./hardware/subsystems/CustomizeHowto_en.md)
- [Switches subsystem](./hardware/subsystems/Switches/Switches_en.md)

## Improvements to the `inputs` namespace

The major improvement is in the way you assign input numbers to switches.
Switch *indices* are not used anymore. They were a nightmare to understand.
Now, some `inputs::add*()` functions will return an object.
You call `inputNumber()` in that object in order to specify an input number for each switch
(which should be called many times).
Those calls are chainable. For example:

```c++
inputs::addAnalogMultiplexer(mpxSelectors, mpxInputs)
    .inputNumber(mpxInputs[0], mux8_pin_t::A0, JOY_BACK)
    .inputNumber(mpxInputs[0], mux8_pin_t::A1, JOY_LB)
    .inputNumber(mpxInputs[0], mux8_pin_t::A2, LCLUTCH)
    .inputNumber(mpxInputs[0], mux8_pin_t::A3, JOY_A)
    .inputNumber(mpxInputs[0], mux8_pin_t::A4, RCLUTCH)
    .inputNumber(mpxInputs[0], mux8_pin_t::A5, JOY_B)
    .inputNumber(mpxInputs[0], mux8_pin_t::A6, JOY_RB)
    .inputNumber(mpxInputs[0], mux8_pin_t::A7, JOY_START)
    .inputNumber(mpxInputs[1], mux8_pin_t::A0, 10)
    .inputNumber(mpxInputs[1], mux8_pin_t::A1, 8)
    .inputNumber(mpxInputs[1], mux8_pin_t::A2, JOY_X)
    .inputNumber(mpxInputs[1], mux8_pin_t::A3, 12 )
    .inputNumber(mpxInputs[1], mux8_pin_t::A4, JOY_Y)
    .inputNumber(mpxInputs[1], mux8_pin_t::A5, 13)
    .inputNumber(mpxInputs[1], mux8_pin_t::A6, 9)
    .inputNumber(mpxInputs[1], mux8_pin_t::A7, 11);
```

The arguments to `inputNumber()` depends on the particular hardware:

- *Button matrix*: you specify the selector and input pins where the switch is attached.
- *Analog multiplexer*: you specify the input pin coming from the multiplexer and a pin tag at that multiplexer.
- *Shift register*: you specify the position of the chip in the chain and a pin tag at that chip.
- *GPIO expander*: you specify a pin tag.

Another improvement is in the way you write arrays of GPIO pins.
Now there is no need to deal with sizes or lengths.
Just write GPIO pins between brackets.
For example:

```c++
auto &matrix = inputs::addButtonMatrix(
    {GPIO_NUM_4, GPIO_NUM_16, GPIO_NUM_17},
    {GPIO_NUM_15, GPIO_NUM_5,GPIO_NUM_21});
```

The firmware version 4 also changes some behaviors:

- It does not require you to assign an input number to every switch.
  Any switch with no input number assigned will do nothing.
  However, unlike previous versions, the firmware will boot normally.
- Unlike previous versions, it is allowed to assign the same input number to two or more switches.
  However, there is no point in that. The firmware will not warn you.

## Improvements to the `inputHub` namespace

*Bitmaps* are not used anymore.
In order to specify a combination (or list) of inputs, put the input numbers (or aliases) between brackets.
For example:

```c++
inputHub::setALTInputNumbers({1, 2});
inputHub::cmdRecalibrateAnalogAxis_setInputNumbers({JOY_LSHIFT_PADDLE, JOY_RSHIFT_PADDLE, JOY_START});
```

Note that there are changes in the name of some API calls:

| Before                                 | Now                                          |
| -------------------------------------- | -------------------------------------------- |
| setALTBitmap()                         | setALTInputNumbers()                         |
| setALTButton()                         | *(removed)*                                  |
| setClutchCalibrationButtons()          | setClutchCalibrationInputNumbers()           |
| cycleALTButtonsWorkingMode_setBitmap() | cycleALTButtonsWorkingMode_setInputNumbers() |
| cycleCPWorkingMode_setBitmap()         | cycleCPWorkingMode_setInputNumbers()         |
| cycleDPADWorkingMode_setBitmap()       | cycleDPADWorkingMode_setInputNumbers()       |
| cpWorkingMode_setBitmaps()             | cpWorkingMode_setInputNumbers()              |
| cmdRecalibrateBattery_setBitmap()      | cmdRecalibrateBattery_setInputNumbers()      |
| cmdRecalibrateAnalogAxis_setBitmap()   | cmdRecalibrateAnalogAxis_setInputNumbers()   |
| setCalibrationCommandBitmaps()         | *(removed)*                                  |
