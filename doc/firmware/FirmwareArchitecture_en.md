# Firmware (software) architecture

## Modules

The *system* have been broken into several *modules* that have been implemented as C++ namespaces.
All of them are defined at *SimWheel.h*:

- **batteryCalibration**: Everything related to battery profiling.
- **batteryMonitor**: Everything related to the measurement of available battery charge.
- **capabilities**: Everything related to the capabilities of the hardware and firmware.
- **hidImplementation**: Everything related to the HID protocol.
- **hidImplementation::common**: Common behavior for all HID implementations (USB and BLE).
- **inputs**: Everything related to hardware inputs and their events.
- **inputHub**: Everything related to the combined state of all inputs and their treatment.
  Translates input events into a HID report.
- **notify**: Everything related to the notification of some events to the user if an user interface is available.
- **power**: Everything related to power management.
- **userSettings**: Manages user settings and their long term storage.

Each namespace is implemented in a single *cpp* file with its name, however,
some of them have alternate implementations in order to enable unit and integration testing.
Those files are named following this pattern: `<namespace>_<implementation>.cpp`.
Some implementations are:

- *mock*: dummy implementation with no actual behavior.
- *serial*: implementation providing output to the serial port.

All modules can be found at the [/common](../../src/common/) folder.

### Auxiliary modules

Some namespaces are implemented with the help of auxiliary modules which are not exposed at *SimWheel.h*, one *cpp* file for each:

- *adcTools*: Reading of ADC pins.
- *AnalogMultiplexerInput*: Everything related to multiplexed buttons/switches.
- *ButtonMatrixInput*: Everything related to button/switch matrices.
- *debugUtils.cpp*: Minor utilities for debugging and testing.
- *PolledInput*: Related to inputs that must be read in a polling (or sampling) loop.
  Defines two main c++ classes: `AnalogAxisInput` and `DigitalPolledInput`.
- *RotaryEncoderInput*: Everything related to rotary encoders.
- *SerialNotification*: For the testing of user notifications through the USB serial interface.
- *ShiftRegistersInput*: Everything related to serialized buttons/switches.
- *i2cTools*: I2C bus initialization and common utilities.
- *I2CExpanderInput*: Everything related to GPIO expanders on the I2C bus.

### Principle of single responsibility

| Module                 | Reason to change                                        |
| ---------------------- | ------------------------------------------------------- |
| adcTools               | Requirements to ADC readings (for example, attenuation) |
| AnalogAxisInput        | Hardware design                                         |
| AnalogMultiplexerInput | Hardware design                                         |
| batteryCalibration     | SoC algorithm                                           |
| batteryMonitor         | Hardware for SoC measurement                            |
| ButtonMatrixInput      | Hardware design                                         |
| capabilities           | Hardware and firmware features relevant to the user     |
| hidImplementation      | Device-computer intercommunication                      |
| I2CExpanderInput       | Hardware design                                         |
| i2cTools               | I2C API                                                 |
| inputHub               | Device functionality                                    |
| inputs                 | Input hardware                                          |
| notify                 | User interface hardware (if any)                        |
| power                  | Underlying power management capabilities                |
| RotaryEncoderInput     | Hardware design                                         |
| ShiftRegistersInput    | Hardware design                                         |
| userSettings           | Long term storage of user settings                      |

### Module dependencies

Only most relevant information is shown below:

```mermaid
classDiagram
    class inputs {
      +start()
    }
    class AnalogAxisInput
    class inputHub {
      +onRawInput()
    }
    class hidImplementation {
      +reportInput()
    }
    class power {
      +powerOff()
    }
    class batteryCalibration {
      +getBatteryLevel()
      +getBatteryLevelAutoCalibrated()
    }
    class DigitalPolledInput {
      +read()
    }
    class batteryMonitor {
        + getLastBatteryLevel()
    }
    class userSettings {
      +bitePoint
      +cpWorkingMode
      +altButtonsWorkingMode
    }
    inputHub <-- inputs: input events
    inputs <-- PolledInput: state of input hardware
    inputs <-- AnalogAxisInput: axis position
    PolledInput <|-- DigitalPolledInput
    inputHub --> hidImplementation: processed events
    inputHub <--> userSettings: configuration
    hidImplementation <--> userSettings: configuration
    hidImplementation --> power: auto power-off
    hidImplementation <-- batteryMonitor: current battery level
    batteryMonitor --> batteryCalibration: battery voltage
    batteryMonitor <-- batteryCalibration: computed battery level
    batteryMonitor --> power: power-off on critical battery level
```

[Render this graph at mermaid.live](https://mermaid.live/view#pako:eNqdVMtu2zAQ_BWCpxaNf0AIAjhNgQZI0KA59KLLWlzJRClSIJdxg9T_3tXDAiWqQVFfTC1nZl8jvcnKKZSFrAyEcKeh8dCWVvBviAhtu0hBvI0xIT4FAk8fPo7P5xS6t2Bcs_-lw31PylS-xkOi4-x3OA3ATbGjVvdtZ7BFS0Da2YTqsXOe_s7t3Al9gh-ev9X1JvgAROhfP4PRB7_O1CDdjvcP-ILmIpDf7CO5iwSqzUR3utEE5skZg2ooftESqPfKe3RWk0uaYo7gGh4gbFa40IgB_TMSadukmzxowienLc2Rqvvh_E-GPbIp5igYuo1Ezobsdkozr_d6t5scU4z_gouyFBJYGEDJFArBliIUrp4oR_DqBB4z0spghQA-8raD7tc2wtPxXv9mUj72Vc273U3utkJ03lUYAqq8hanTm8VgC1E5W-smjiYa4bmL_5fX0wYfc9fstfG8c3X9TqKVeThT9J4Bl7gwvWVGgZXP-nT5m1HMzBdnCBrc5CaZF9zKtTw8Hui_pZ-6nRsV3FTledUVmExCXskWfQta8bdssHgp6cjjKGXBR4U1REOlLO2Zof0En19tJQvyEa9k7BQ7cPr6LYNfVF_QGDv_Abn-wmA)

```mermaid
classDiagram
    class I2CButtonsInput{
      #getGPIOstate()
    }
    DigitalPolledInput <|-- ButtonMatrixInput
    DigitalPolledInput <|-- AnalogMultiplexerInput
    DigitalPolledInput <|-- ShiftRegistersInput
    DigitalPolledInput <|-- RotaryEncoderInput
    DigitalPolledInput <|-- I2CInput
    I2CInput <|-- I2CButtonsInput
    I2CButtonsInput <|-- PCF8574ButtonsInput
    I2CButtonsInput <|-- MCP23017ButtonsInput
```

[Render this graph at mermaid.live](https://mermaid.live/view#pako:eNqNkVFLwzAUhf9KuL4obKBTmRRftJvSh2LZXvtyaW67QJqU5AY2Zv-7sdVRfJDmJcm530ngnDNUVhIkUGn0fqOwcdiWRsQ1KCJbpa-B2RqfmS7weZwJcdUQvxfZh2dkur4Z5X7cNqpRjLqwWpMcbOL5c7kU40M5slPHQf4ffzGobZMHzarTdCQ3w7M_qJp31CjP5PwMw84yutPWfMcw54OYx4T6vV1m06wuyFQcySJ9e3pcP8yk87RY3d_eracTWEBLrkUlY3lDKyXwgVoqIYlHSTXG4EooTR9RDGz3J1NBUqP2tIDQydjbT99_1K1UbB0k7AL1X8JeuZc)

```mermaid
classDiagram
    hidImplementation --> inputs: command to calibrate analog axes
    hidImplementation --> batteryCalibration: command to recalibrate battery
    hidImplementation --> notify: connected, discovering, page control
    batteryMonitor --> notify: low battery level
    userSettings --> notify: bite point
```

[Render this graph at mermaid.live](https://mermaid.live/view#pako:eNp9kLFuwzAMRH-F0Oz8gIcubYYOnbJ6oSXGISCRhkQlNYL8e2TEReElmoTj3cORd-c1kOudj1jKF-OUMQ0C7V04fKc5UiIxNFaBw-EDWOZqpQevKaEEMAWPkceMRoCCUSfAXyrvGCOaUV4-t1yTd7xM_8TN-o4manxeVoIIeaPQQeDi9UqZZepgxonWoWWNL84G_VFh07yDRL39jSHSlbZELZRPZNaAZecfuZWclcVc5xLlhBzaNe9rbHB2aV0H17dvoDPWaIMb5NGsWE1Pi3jXW67UuTqHtu52_714DGvLl_Z4AoBnmLs)

Some modules have a `begin()` method that must be called at system startup (`main()`or `setup()`).
The calling order is defined by the previous diagram, where bottom modules must be called first.

### Definitions (header files)

All header files can be found at the [/include](../../src/include/) folder.
Most relevant are:

- **SimWheel.h**: definition of all modules (namespaces).
- **SimWheelTypes.h**: common constants and types for all modules.
- **debugUtils.h**: constants and utilities for unit testing.

## Brief description of most relevant modules

For detailed description, see the Doxigen's documentation at [SimWheel.h](../../src/include/SimWheel.h).

### DigitalPolledInput and descendant classes

There is a dedicated daemon that read the state of those inputs in a loop, every few milliseconds.
Since many inputs are read at the same time, the combined state of all of them is reported to `inputHub`.
Nothing is reported if there are no input events, this is, a state change since the previous iteration.

### AnalogAxisInput

It works in a similar way to `DigitalPolledInput`, but for analog inputs,
which are limited to two clutch paddles with potentiometers.

### Inputs

This is the place where inputs are set up and a number assigned to them.
Use the `add*()` methods to set up any kind of inputs (button matrices, rotary encoders, etc) and their pins.
Analog clutch paddles are set here, too.

The assigned input number will be mapped to a user-defined HID button number,
which will be reported to the hosting computer when needed.
If there is no user-defined map, it defaults to the following rule:

```text
if (alt mode engaged) then
   HID button number = raw input number + 64
else
   HID button number = raw input number
```

### InputHub and userSettings

Almost all the logic behind the behavior of the sim wheel is implemented at these modules.
Wheel's functions are mapped to input numbers at `InputHub`.

### Capabilities

This module holds static data about device capabilities.
For example, it tells if the device has clutch paddles or not.
Such data is set from other modules at startup.
This module is trivial, so it is not shown in the previous diagram.
It may be called from any other module.

### Notifications

This module provides a generic way to notify events to the user, if a user interface is available.
It does not depend on a particular hardware, so, anything could be implemented in the future:
a single LED, sounds, an OLED, etc.
By default, it does nothing.
To provide a particular user-interface implementation,
derive a new class from `AbstractNotificationInterface`,
then provide an instance to `notify::begin()`.

All notifications are queued, serialized and executed in a very low priority separate thread.
The calling thread does not wait for them.
For those reasons, some notifications may be missed.

If there were two or more user interfaces (for example, display and sounds),
they should be implemented in separate classes, not to mix their code.

`AbstractNotificationInterface` may work in two, non-exclusive, modes:

#### As a simple message queue

For user interfaces not needing a perpetual loop or for one-time notifications. This is the default behavior.
For example:

```c
   void MyImpl::onStart() {
      turnLedOn();
   }

   void MyImpl::onConnected() {
      // blink 2 times
      delay(250);
      turnLedOff();
      delay(250);
      turnLedOn();
      delay(250);
      turnLedOff();
      delay(250);
      turnLedOn();
   }
```

#### As a frame server

For user interfaces in need of a perpetual loop or for persistent notifications.
If there are no pending notifications,
`AbstractFrameServerInterface::serveSingleFrame()` will be called at timed intervals.
A non-zero frames-per-second value must be given to `notify::begin()`.
For example:

```c++
   void MyImpl::onStart() {
     discovering = false;
   }

   void MyImpl::onBLEdiscovering() {
      discovering = true;
   }

   void MyImpl::onConnected() {
      discovering = false;
      turnLedOn();
   }

   void MyImpl::serveSingleFrame() {
    // Called one time per second
    if (discovering)
      switchLed();
   }

   ...

   void setup()
   {
      ...
      notify::begin({new MyImpl(ledPin)}, 1); // FPS = 1
   }
```

*Note*: `AbstractFrameServerInterface::onLowBattery()` is already
called at timed intervals as long as such a condition persists.

### BatteryMonitor

This module is in charge of interfacing the underlying hardware for "state of charge" (SOC) estimation.
A daemon computes SOC in timed intervals under this algorithm:

1. Measure some battery property, depending on the underlying hardware:

   - *Voltage divider* or *battery monitor*: indirect voltage.
   - *Fuel gauge*: state of charge.

2. Determine if the battery is attached or not.
3. Compute state of charge:

   - *Voltage divider* or *battery monitor*:
     the *BatteryCalibration* module translates a voltage into a state of charge.
   - *Fuel gauge*: computation is already done by the chip itself.

4. Notify low battery levels.
5. Power off on very low battery levels.

#### Fuel gauges

Fuel gauges from Maxim/Analog Devices are powered from the battery itself.
As a result, they don't respond to I2C commands if the battery is not attached.
This way, we know there is no battery.

### BatteryCalibration

Provides an estimation of the "state of charge" given an indirect battery voltage.

#### Most accurate algorithm

Battery calibration is required for accurate battery levels.
Calibration goes from full charge to battery depletion,
taking a sample of battery voltage every minute.
All possible voltages are divided into 32 evenly distributed ranges, called *quantum*.
Calibration data is just a set of counters of voltage samples for each quantum.
The sum of all counters is equivalent to 100% battery charge.
Calibration data is stored in flash memory.

Let's be $V_{min}(i)$ the minimum voltage that falls into quantum $i$ (a natural number),
$a < b \iff V_{min}(a)<V_{min}(b)$.
Let's be $QSIZE = V_{min}(i+1)-V_{min}(i)+1$ (the same for every quantum).
Let's be $S(i)$ the count of samples for quantum number $i$.
Let's say we have a battery voltage $V_n$ that falls into the quantum number $n$ (0-index).

$BatteryLevel = \frac{ (\sum_{i=0}^{n-1}S(i)) + \frac{S(n)*(V_n-V_{min}(n))}{QSIZE} }{ \sum_{j=0}^{31}S(j) } * 100$

Note that "most accurate" does not mean "accurate".
Battery voltage is not enough for accurate SOC.

#### Auto-calibrated algorithm

If calibration data is not available,
a rough estimation is provided based on LiPo batteries characterization data taken from here:
[https://blog.ampow.com/lipo-voltage-chart/](https://blog.ampow.com/lipo-voltage-chart/).
However, actual battery voltages may not match the characterization data due to:

1) inaccurate ADC readings,
2) voltage drop due to the involved transistors (if any) and
3) unexpected impedances at the voltage divider.

For this reason, the highest voltage ever read is taken as an auto-calibration parameter.
The expected voltage reading is mapped linearly to the absolute maximum voltage ever read.
The battery needs a full charge before this algorithm provides any meaningful result.

(See [LiPoBatteryCharacterization.ods](./LiPoBatteryCharacterization.ods))

### HidImplementation

All data interchange between the device and the host computer is conducted through the HID protocol.
This involves:

- State of buttons, axes and the alike.
- Device capabilities.
- Configuration: clutch paddles, "ALT" buttons, battery calibration, etc.

See [HID notes](./HID_notes.md) for more details.

## About digital inputs and input events

Every hardware input is assigned a single number starting from 0 and up to 63.

The state of an input is represented by a single bit, 1 meaning a pressed button, 0 meaning a released button.
So, the combined state of all inputs is represented as a 64-bits word,
where the n-th bit represents the n-th input number.
This is called an *input bitmap*.
Least significant bit is numbered as zero.
For example, the word `00000000 ... 00000101` means that buttons 0 and 2 are pressed,
and the others are released.

An input can also be identified by a bitmap. For example, the button number 3 can be expressed as `...01000`.

### Input masks

A mask is a 64-bits word where each bit represent an input number (the same as input bitmaps),
but a bit set to 1 means that an input is not set and *must be ignored* in the corresponding bitmap.
Input masks are used in combination with bitmaps to build a combined state.
For example:

| State              | Bitmap   | Bitmask  |
| ------------------ | -------- | -------- |
| (A) previous state | 11000001 | 11111111 |
| (B) partial state  | 00000010 | 11111100 |
| (C) new state      | 11000010 | 11111111 |

where `bitmap(C) = bitmap(B) OR (bitmask(B) AND bitmap(A))` being AND/OR bitwise operators.

### Event processing

Input events are captured in the **Input poll daemon**:
it checks the state of all inputs every 50 ms (more or less).
This period is short enough not to miss any event,
but long enough to prevent other threads from starvation.
It also plays a critical role in debouncing
(bouncing occurs during the subsequent 30 milliseconds, more or less,
after a mechanical switch is activated).

Event processing takes long, so later input events would be missed while processing sooner ones.
To prevent this, input events are posted into a queue.

```mermaid
flowchart LR
  IP((Input poll daemon))
  Q[input event queue]
  IH((Input hub daemon))
  IP -- event --> Q
  Q -- event --> IH
```

[Render this graph at mermaid.live](https://mermaid.live/view#pako:eNpVjssOgjAQRX-lmRUk9AdYuNKEJi5Al9ZFpYM06QObqcYQ_l1AXbCbnHvuzYzQBo1QQmfDq-1VJHY8Sc-YqLNM-CERG4K1TCt0wef5EjUXswb4RE_skTDhda1U_0qfbpuGqBnnP5_zHWvWmS0TFRTgMDpl9PzPuCgSqEeHEsr51NipZEmC9NOsqkTh_PYtlBQTFpAGrQj3Rt2jclt40IZC_LLpAwzyT0k)

Event capture is detached from event processing at the **input hub daemon**,
which runs most of the code. Note that such a daemon is implemented inside `inputs.cpp`,
not `inputHub.cpp`.

Raw inputs are transformed into a HID input report in a sequence of "filters" or steps:

1. Detect and execute user commands, if any.
   If a command is detected and executed, this sequence is interrupted.
2. Depending on user settings, transform analog axis input into buttons input or vice-versa.
3. Execute clutch bite point calibration when requested.
4. Determine if ALT mode is engaged.
5. Compute F1-style clutch position.
6. Transform DPAD inputs into navigational input, depending on user settings.
7. Map raw button inputs into user-defined inputs, if any, or use default mapping.

#### A note on rotary encoders

Each detent of a rotary encoder generates two input events in quick succession:
a button press and then, release.
Decoding is implemented by hardware interrupts,
but input events are read in the *input poll daemon*.
In summary:

1. `DT` and `CLK` signals are decoded in an interrupt service routine.
   If a rotation event is detected (clockwise or counter-clockwise),
   that event is pushed into a simple bit-oriented queue.
2. The *input poll daemon* extracts an event from the queue,
   then modifies the state of the corresponding button as pressed.
3. At the next iteration, it will reset the state of that button as non-pressed,
   thus simulating a press-then-release sequence of events.

The bit-oriented queue shows the following properties:

- Implemented as a [circular buffer](https://en.wikipedia.org/wiki/Circular_buffer).
- Thread-safe.
- Unnoticeable memory footprint.
- Size for 64 rotation events.
  If the queue were full, latest events would be discarded.
- Since each rotary is polled every 50 ms,
  it is unlikely for the queue to get full.

## About auto power off

In a battery-operated system, when there is no Bluetooth connection,
the systems goes to advertising.
If no connection is made in a certain time lapse,
the system goes to deep sleep or power off.

## About connectivity

The firmware relies in the [HID](https://en.wikipedia.org/wiki/Human_interface_device) standard to provide connectivity.
The device will appear as a [Gamepad](https://en.wikipedia.org/wiki/Gamepad) to the hosting computer.
The *hidImplementation* namespace is in charge of that.
However, this project provides several alternate implementations:

- *hidImplementation_NimBLE.cpp*: BLE using the
  [NimBLE stack](https://mynewt.apache.org/latest/network/).
  Requires an additional Arduino
  [library](https://www.arduino.cc/reference/en/libraries/nimble-arduino/).
- *hidImplementation_ESPBLE.cpp*: BLE using the native ESP-Arduino stack.
  Does not require additional libraries, but it takes way more flash memory than *NimBLE*.
- *hidImplementation_USB.cpp*: wired USB implementation.

## Concurrency

System concurrency comes from these OS task and daemons:

- *Main task*: Performs initialization, then goes dormant.
- *Input poll daemon*. May call:
  - `inputs` and auxiliary modules.
- *Input hub daemon*. May call:
  - `inputHub`
  - `hidImplementation`
  - `inputs`
  - `userSettings`
  - `batteryCalibration`
  - `notify`
- *Battery monitor daemon*. May call:
  - `power`
  - `batteryCalibration`
  - `hidImplementation`
  - `notify`
- *OS timers*. May call:
  - `inputs`
  - `userSettings`
  - `notify`
- *Bluetooth/USB stack*. May call:
  - `hidImplementation`
  - `userSettings`
  - `inputs`
  - `batteryCalibration`
  - `notify`
  - `batteryMonitor`
