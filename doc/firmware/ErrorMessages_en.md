# Help on error messages

## How-to

Error messages are **not** shown by default.
In order to enable them, go to the "board manager" and configure "Core debug level" to "error".
Then, upload the firmware again (if not done yet).

Open the "Serial monitor" at Arduino IDE to view error messages.
Then, reset the board not to miss any message at boot-up.

Many error messages are provided by the operating system or system libraries.
Look for those error messages on the Internet.
This article is focused on **firmware-defined** error messages.

## Firmware-defined error messages

- *getADCreading: GPIO N is not ADC*

  A GPIO pin given to `inputs:setAnalogClutchPaddles()` or `batteryMonitor::begin()`
  is not ADC-capable. Check your custom firmware.

- *No input/selector pins at AnalogMultiplexerInput::AnalogMultiplexerInput()*.

  An empty array was given to `inputs::addAnalogMultiplexer()`.
  Check your custom firmware.

- *Invalid input pin at AnalogMultiplexerInput::inputNumber()*.

  An input number is not in the range from 0 to 63 in `inputs::addAnalogMultiplexer().inputNumber()`.
  Check your custom firmware.

- *Not enough memory at AnalogMultiplexerInput::AnalogMultiplexerInput()*
- *Not enough memory at ButtonMatrixInput::ButtonMatrixInput()*

  Should not happen. Please, open an issue
  and provide details on your DevKit board.

- *parameter out of range: batteryCalibration::addSample()*
- *Logic error at batteryCalibration::addSample()*
- *Logic error at batteryCalibration::getBatteryLevel()*

  Should not happen. Please, open an issue.

- *Too few/many input or selector pins at ButtonMatrixInput::ButtonMatrixInput()*

  Either an empty array was given to `inputs::addButtonMatrix()` or there are more than eight GPIO pins
  at the input or selector arrays. Check your custom firmware.

- *Invalid selector pin at ButtonMatrixInput::inputNumber()*

  An input number is not in the range from 0 to 63 in `inputs::addButtonMatrix().inputNumber()`.
  Check your custom firmware.

- *Unable to create HID device*
- *Unable to create HID report characteristics*

  Should not happen. Please, open an issue.

- *Invalid input number N in a call to the inputHub namespace*

  An input number is not in the range from 0 to 63 in some call to `inputHub::...`. Check your custom firmware.

- *Invalid input numbers at inputHub::setClutchInputNumbers()*

  An input number is not in the range from 0 to 63 in `inputHub::setClutchInputNumbers()`.
  Check your custom firmware.

- *Invalid input numbers at inputHub::setClutchCalibrationInputNumbers()*

  An input number is not in the range from 0 to 63 in `inputHub::setClutchCalibrationInputNumbers()`.
  Check your custom firmware.

- *inputs::add() or inputs::set() called after inputs::start()*

  You are not allowed to add more hardware inputs after `inputs::start()`.
  Change the order of those calls.

- *inputs::setAnalogClutchPaddles() called twice or for two identical GPIO pins*

  You must provide two *different* GPIO pins to `inputs::setAnalogClutchPaddles()`.
  You must not call `inputs::setAnalogClutchPaddles()` twice.
  Check your custom firmware.

- *No GPIO expander found with hardware address X (hex)*

  A GPIO expander is not powered or the given *hardware address* is wrong.
  Check your wiring, then your custom firmware.

- *Unable to auto-detect full address of GPIO expander. Hardware address is N (hex)*

  The printed hardware address in not unique. You must specify a *full address* instead.
  In case you have no clue about what the full address could be,
  upload and run the [I2C probe](../../../../src/Firmware/I2C_probe/I2C_probe.ino) firmware
  provided in this project.

- *Unable to create event queue*
- *Unable to create inputHub task*
- *Unable to create polling task*

  Should not happen. Please, open an issue.

- *Unable to create notifications daemon*

  Should not happen. Please, open an issue.

- *Requested GPIO N can't be used as output*

  In a call to `inputs::...` you are passing a GPIO pin which is not output-capable.
  Check selector, `next`, `load` and/or I2C pins in your custom firmware.

- *Requested GPIO N can't be used as input*

  In a call to `inputs::...` you are passing a GPIO pin which is not input-capable.
  Check input pins in your custom firmware.

- *Invalid input number N (not in range 0-63)*

  In a call to `inputs::...()` you are passing an input number which is not in the range from 0 to 63.
  Check your custom firmware.

- *AnalogAxisInput::AnalogAxisInput: given pins are not usable*

  A GPIO pin given to `inputs:setAnalogClutchPaddles()` is not ADC-capable.
  Check your custom firmware.

- *I2C device not found at address N, bus=M*

  A GPIO expander is not powered or the given *I2C address* is wrong.
  Check your wiring, then your custom firmware.
  Run the [I2C probe](../../../../src/Firmware/I2C_probe/I2C_probe.ino) firmware
  provided in this project to reveal all I2C (full) addresses.

- *Too many buttons at GPIO expander. Address=N, bus=M*

  Should not happen. Please, open an issue.

- *power::begin(): Invalid rtc pin for wake-up (N)*

  The GPIO pin given to `power::begin()` for wake-up is not RTC-capable.
  Choose an RTC-capable pin. Look for a data sheet or pin-out diagram.

- *power::setPowerLatch(): no valid GPIO = N*

  The GPIO pin given to `power::setPowerLatch()` is not a valid output-capable pin.
  Choose a proper pin.

- *power::powerOff(): Deep sleep not working*

  Should not happen. Please, open an issue.

- *batteryMonitor::begin(): given pins are not usable*

  Either parameter `battENPin` is not a valid output-capable pin,
  or parameter `battREADPin` is not a valid ADC-capable pin at `power::startBatteryMonitor()`.
  Check your custom firmware.

- *batteryMonitor::begin(): unable to start daemon*

  Should not happen. Please, open an issue.

- *clkPin and dtPin must not match in RotaryEncoderInput::RotaryEncoderInput()*

  You must provide *different* `DT` and `CLK` pins to `inputs::addRotaryEncoder()`.
  Check your custom firmware.

- *Too few/many switches at ShiftRegistersInput::ShiftRegistersInput()*

  The parameter `switchCount` to `inputs::addShiftRegisters()` must be greater than zero and lower than 65.
  Check your custom firmware.

- *Unable to allocate memory for input bitmaps at ShiftRegistersInput::ShiftRegistersInput()*

  Should not happen. Please, open an issue.

- *Invalid position of input number X at shift register index N and pin M*.

  You are trying to assign an input number to a switch which does not exist in the shift registers chain.
  Check both the `switchCount` parameter to `inputs::addShiftRegisters()`
  and the subsequent `.inputNumber()` calls.

- *Not a valid I2C address: X (hex)*.

  You are trying to use an invalid full I2C address in 7-bit format.
  Valid I2C addresses are in the range from 0 to 127, inclusive.
  Check calls to `inputs::addMCP23017Digital()`, `inputs::addPCF8574Digital()` and
  `batteryMonitor::begin()`.
  Note that some data sheets specify the I2C address in 8-bit format.
  In such a case, displace all bits one position to the right in order to obtain the address in 7-bit format.

- *Unable to initialize I2C bus N, SDA=A, SCL=B, clock multiplier=C*

  The given pins to `inputs::initializeI2C()` are unable to work as an I2C bus.
  Try other pins.
  Ensure they have an internal or external pull resistor.
  Ensure they are both input-capable and output-capable.
  If such a call was not placed in your custom firmware,
  open an issue.

- *notify::begin() called with an empty set of implementors*

  You called `notify::begin( {}, ...)`.
  You must provide a non-empty array of implementors for the abstract notification interface.

- *notify::begin() called with a null pointer implementor*

  A null pointer is not allowed in the array of implementors given to `notify::begin()`.
