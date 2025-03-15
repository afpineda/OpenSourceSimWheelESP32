# Glossary of terms and definitions

- **ADC**: Analog to Digital Converter.

- **Alternate mode** (or "ALT mode"):
  a function for certain buttons.
  When enabled, other input numbers will be reported as `input number + 64` (configurable).
  This duplicates the number of reported inputs in the _HID_ protocol.

- **Bite point**:
  a certain value of the _clutch_ where the simulated car
  begins to transfer torque from the engine to the wheels.

- **BLE**: Bluetooth Low Energy.

- **Button**: a momentary switch.

- **Clutch**: a joystick/gamepad axis that simulates the position of a clutch pedal.

- **Daemon**: a perpetual task that runs an infinite loop in a background thread.

- **Detent**: haptic user feedback in a rotary encoder.

- **DPAD** (or **Directional pad**):
  any hardware able to send "up", "down", "left" and "right" _input events_.
  For example: 4 push buttons arranged in a cross
  or a single lever that can move in two axis.

- **Frame**: a graphic that fills the whole display.

- **Frame server**:
  a perpetual task that computes _frames_
  and then shows them in a display, in quick succession.

- **HID**:
  Human Interface Device.

- **HID reports**:
  Data sent to a computer (or received from) using the standard HID protocol.

- **Input**:
  Any hardware able to generate _input events_ caused by user interactions.
  For example: momentary switches and rotary encoders.

- **Input bitmap**:
  A 64-bits word where each bit represents the state of an input.
  An input bitmap with a single bit set to 1 may be used to identify a single input.

- **Input bitmask**:
  a 64-bits word that represents ignored inputs in a bitmap.

- **Input event**:
  Any user action that the _system_ must be aware of.
  For example: the press or release of a momentary switch
  or the rotation of a single _detent_ in a rotary encoder.

- **Input hub**:
  A _subsystem_ that knows the state of every input at all times.

- **Input number**:
  A non-negative number assigned to a hardware input.

- **LiPo** (battery):
  Lithium-polymer battery.

- **Paddle**:
  A momentary switch or a potentiometer shaped as a lever.

- **PISO**:
  Parallel-In Serial-Out (shift register).

- **Polling loop**:
  A perpetual task that reads the state of some inputs
  and generates input events from state changes.

- **Quantum**:
  Talking about battery charge,
  a counter of voltage samples that falls into a certain range.

- **Subsystem**:
  A piece of the _system_ that provides a certain behavior
  by grouping related functions.

- **System**:
  The combination of hardware and software subject of this project.

- **State of charge (SOC)**:
  An estimation of available battery charge in the range from 0% to 100%.

- **UI**: User interface.
