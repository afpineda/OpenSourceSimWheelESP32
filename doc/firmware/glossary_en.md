## Glossary of terms and definitions

- **Alternate mode** (or "ALT mode"): a function for certain buttons. When enabled, other input numbers will be reported as `input number + 64`. This duplicates the number of reported inputs in the _HID_ protocol.

- **AT commands**: a text-based protocol developed by _Hayes_. See [An Introduction to AT Commands](https://www.twilio.com/docs/iot/supersim/introduction-at-commands) for further reference.

- **Bite point**: a certain value of the _clutch_ where the simulated car begins to transfer torque from the engine to the wheels.

- **BLE**: Bluetooth Low Energy.

- **Button**: a momentary switch.

- **Clutch**: a joystick/gamepad axis that simulates the position of a clutch pedal.

- **Daemon**: a perpetual task that runs an infinite loop in a background thread.

- **Detent**: haptic user feedback in a rotary encoder.

- **DPAD** (or **Directional pad**): any hardware able to send "up", "down", "left" and "right" _input events_. For example: 4 push buttons arranged in a cross or a single lever that can move in two axis.

- **Frame**: a graphic that fills the whole display.

- **Frame server**: a perpetual task that computes _frames_ and then shows them in a display, in quick succession.

- **HID**: Human Interface Device.

- **HID messages**: data sent to a computer using the standard HID protocol.

- **Input**: Any hardware able to generate _input events_ caused by user interactions. For example: momentary switches and rotary encoders.

- **Input bitmap**: a 64-bits word where each bit represents the state of an input. An input bitmap with a single bit set to 1 may be used to identify a single input.

- **Input bitmask**: a 64-bits word that represents ignored inputs in a bitmap.

- **Input event**: Any user action that the _system_ must be aware of. For example: the press or release of a momentary switch or the rotation of a single _detent_ in a rotary encoder.

- **Input hub**: a _module_ that knows the state of every input at all times.

- **Input number**: a non-negative and unique number assigned to an input.

- **Module**: a piece of the _system_ that provides a certain behavior by grouping related functions.

- **NuS**: [Nordic UART Service](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/1.6.1/nrf/include/bluetooth/services/nus.html). An implementation of serial communications through BLE.

- **LiPo** (battery): Lithium-polymer battery.

- **Paddle**: a momentary switch shaped as a lever.

- **Polling loop**: a perpetual task that reads the state of some inputs and generates input events from state changes.

- **Quantum**: talking about battery charge, a counter of voltage samples that falls into a certain range.

- **System**: the combination of hardware and software subject of this project

- **UART**: Universal Asynchronous Receiver / Transmitter

- **UI**: User interface
