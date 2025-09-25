# Troubleshooting

When troubleshooting, always make sure you are using the tested versions of the
[external dependencies](./skills_en.md) first.

## How-to

By default, only error messages directly caused by this firmware can be viewed.
To view these error messages, a USB serial port connection is required.
Open the *Serial Monitor* in the Arduino IDE
(hit `ctrl`+`shift`+`M`),
then reset your DevKit board to avoid missing any messages.

> [!IMPORTANT]
> You must configure the serial monitor to **115200 bauds**.
> Otherwise you will not see any error messages.

Other error messages are **not** displayed by default.
In order to enable them, go to the *Board Manager*
and set "Core debug level" to "error".
After that, upload the firmware again.
These additional error messages are caused by the operating system or system libraries.
Check the internet for a description.

This article focuses on **firmware-defined** error messages.

If you are using USB connectivity,
but your DevKit board does not have a second USB serial port,
you can temporarily switch to
["dummy" connectivity](./hardware/subsystems/CustomizeHowto_en.md#connectivity)
for troubleshooting purposes.

## Firmware-defined error messages

Error messages are preceded by the message `**CUSTOM FIRMWARE ERROR**`
and repeated every few seconds.

### Error messages caused by your custom code

- *std::bad_cast*

  You are using a non-initialized variable of
  type `InputNumber` or `UserInputNumber`,
  or passing an invalid value to an input number parameter.
  Valid values are in the range [0,64] for firmware-defined input numbers,
  and [0,127] for user-defined input numbers.

- *Battery monitor already started*

  You called `batteryMonitor::configure()` while
  the firmware is running. This is not allowed.

- *Battery monitor already configured*

  You called `batteryMonitor::configure()` twice.
  Just one battery monitor can be configured.

- *The input number ... can not be mapped, since it is not assigned*

  Check the first parameter to `inputMap::add()`.
  You are passing a firmware-defined input number which is not
  assigned to the input hardware (`inputs::add...`).

- *LEDStrip: pixel count can not be zero*

  You are not allowed to pass zero to the third parameter of
  `pixels::configure()`.

- *A pixel group was configured twice*

  You are not allowed to call `pixels::configure()` twice using
  the same pixel group (first parameter).

- *User interface instance is null*

  You are not allowed to pass `nullptr` as the first parameter
  to `ui::add()`.

- *Useless rotary encoder: no input number for clockwise rotation* and
  *Useless rotary encoder: no input number for counter-clockwise rotation*

  You are passing a non-initialized `InputNumber` variable to
  `inputs::addRotaryEncoder()` (third or fourth parameter).

- *I2C: unable to initialize bus ...*

  The given pins to `inputs::initializeI2C()` are not capable for I2C operation.

- *I2C: invalid address ...*

  The number passed as an I2C address is not valid.
  Valid I2C full addresses are in the range [0,127].
  Valid I2C hardware addresses are in the range [0,7].
  Check your calls to
  `inputs::addMCP23017Expander()`,
  `inputs::addPCF8574Expander()`,
  `batteryMonitor::configure()` or
  `ui::add()`.

- *I2C: device not found, but required...*

  A GPIO expander is required by the input subsystem but it was not found
  in the I2C bus.
  - Check your wiring first, including power lines (`3V3` and `GND`).
  - Check that all GPIO expanders are attached to the proper I2C bus
    using the expected `SCL` and `SDA` pins.
  - When using the secondary bus or non-standard GPIO pins,
    ensure there is a call to `inputs::initializeI2C()`.

- *I2C: unable to detect full address...*

  You specified an I2C hardware address, but the system is unable
  to locate the corresponding full address:
  - Check your wiring first as in the previous error message.
  - Ensure two I2C devices are not using the same hardware address.
  - Ensure your custom firmware is passing the hardware address
    matching your actual hardware address configured using
    the `A0`, `A1` and `A2` pins.
    Beware of bit order: `A0` is the least significant bit.
  - Use a full I2C address instead.

- *Invalid position (...) in a coded rotary switch. Valid range is [0,...)*

  Either:

  - You specified a position index out of the range of available positions
    in a coded rotary switch.
  - You are missing one or more input pins.

  For example:

  ```c++
  RotaryCodedSwitch rotsw;
  ...
  rotsw[8] = 16;
  inputs::addRotaryCodedSwitch(rotsw, GPIO_NUM_3, GPIO_NUM_4, GPIO_NUM_5);
  ```

  Since there are 3 input pins, your rotary switch has 8 positions
  in the range [0,7]. Thus, `rotsw[8]` is out of range.

- *"Input numbers used in all coded switches must be unique*

  All calls to `inputHub::codedSwitch::add()` must specify unique
  input numbers in their parameters.

  For example:

  ```c++
  ...
  inputHub::codedSwitch::add(10,11,12,13,sw1);
  inputHub::codedSwitch::add(13,14,15,16,sw2);
  ...
  ```

  is wrong because the input number `13` can not be shared.

When troubleshooting I2C error messages,
the [I2C probe](../src/Firmware/I2C_probe/I2C_probe.ino) firmware
will be handy.

The following error messages are self-describing:

- *You can not assign the same input number for the left and right clutch paddles*
- *You can not assign the same input number for increase and decrease bite point*
- *For neutral gear, a combination of two or more hardware inputs is required*
- *You have analog clutch paddles, but you forgot to call inputHub::clutch::inputs()*
- *There are no analog clutch paddles, but you called cmdRecalibrateAxisInputs()*
- *There are no clutch paddles, but you called inputHub::clutch::bitePointInputs()*
- *There are no clutch paddles, but you called inputHub::clutch::cycleWorkingModeInputs()*
- *There is no DPAD, but you called inputHub::dpad::cycleWorkingModeInputs()*
- *There are no ALT buttons, but you called inputHub::altButtons::cycleWorkingModeInputs()*
- *inputs::setAnalogClutchPaddles() called twice*
- *Unable to add a user interface instance while running*
- *Useless rotary encoder: same input numbers for clockwise and counter-clockwise*

### Internal error messages

The following error messages should not appear.
Please, open an
[issue](https://github.com/afpineda/OpenSourceSimWheelESP32/issues/new?template=bug_report.md).

- Provider not injected to service...
- Logic error at batteryCalibration::getBatteryLevel()
- Parameter out of range: batteryCalibration::addSample()
- Logic error at batteryCalibration::addSample()
- Unable to start the battery monitor daemon
- Input service already started
- Unable to create decoupling queue
- Unable to create inputHub task
- Unable to create polling task
- Unknown pixel driver in LED strip
- Unable to create UI daemon
- Wrong count of input pins in a coded rotary switch
- LEDStrip: rmt_new_tx_channel() failed
- LEDStrip: rmt_new_bytes_encoder() failed

## Non-error messages

Some "error messages" are not errors at all.
You can ignore them:

- *... nvs_open failed ...*
- *E (15) gpio: gpio_install_isr_service(500): GPIO isr service already installed*
- *E NimBLEAdvertisementData: Cannot add UUID, data length exceeded!*
- *E (67) rmt: rmt_new_tx_channel(265): DMA not supported*

## USB not recognized error (ESP32S3 boards)

If your host computer shows an "USB not recognized" error message,
you **forgot to enable the USB implementation**.
See ["Connectivity"](./hardware/subsystems/CustomizeHowto_en.md#connectivity).
