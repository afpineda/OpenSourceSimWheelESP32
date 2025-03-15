# Troubleshooting

## How-to

By default, only error messages directly caused by this firmware can be viewed.
To view these error messages, a USB serial port connection is required.
Open the *Serial Monitor* in the Arduino IDE,
then reset your DevKit board to avoid missing any messages.

> [!IMPORTANT]
> You must configure the serial monitor to **115.200 bauds**.
> Otherwise you will not see any error messages.

Other error messages are **not** displayed by default.
In order to enable them, go to the *Board Manager*
and set "Core debug level" to "error".
After that, upload the firmware again.
These additional error messages are caused by the operating system or system libraries.
Check the internet for a description.

This article focuses on **firmware-defined** error messages.

This article focuses on firmware-defined error messages.

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

## Non-error messages

Some "error messages" are not errors at all.
You can ignore them:

- *... nvs_open failed ...*
- *E (15) gpio: gpio_install_isr_service(500): GPIO isr service already installed*
- *E NimBLEAdvertisementData: Cannot add UUID, data length exceeded!*

## USB not recognized error (ESP32S3 boards)

If your host computer shows an "USB not recognized" error message,
you **forgot to enable the USB implementation**.
See ["Connectivity"](./hardware/subsystems/CustomizeHowto_en.md#connectivity).
