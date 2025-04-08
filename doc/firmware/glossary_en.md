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
  data sent to a computer (or received from) using the standard HID protocol.

- **Input**:
  any hardware able to generate _input events_ caused by user interactions.
  For example: momentary switches and rotary encoders.

- **Input bitmap**:
  a 64-bits word where each bit represents the state of an input.
  An input bitmap with a single bit set to 1 may be used to identify a single input.

- **Input bitmask**:
  a 64-bits word that represents ignored inputs in a bitmap.

- **Input event**:
  any user action that the _system_ must be aware of.
  For example: the press or release of a momentary switch
  or the rotation of a single _detent_ in a rotary encoder.

- **Input hub**:
  a _subsystem_ that knows the state of every input at all times.

- **Input number**:
  a non-negative number assigned to a hardware input.

- **LiPo** (battery):
  Lithium-polymer battery.

- **Paddle**:
  a momentary switch or a potentiometer shaped as a lever.

- **PISO**:
  Parallel-In Serial-Out (shift register).

- **Pixel**:
  a small dot that emits light of any colour.

- **Pixel control**:
  the ability to adjust the color of individual pixels and
  display them all at once.

- **Polling loop**:
  a perpetual task that reads the state of some inputs
  and generates input events from state changes.

- **Quantum**:
  talking about battery charge,
  a counter of voltage samples that falls into a certain range.

- **Subsystem**:
  a piece of the _system_ that provides a certain behavior
  by grouping related functions.

- **System**:
  the combination of hardware and software subject of this project.

- **State of charge (SOC)**:
  an estimation of available battery charge in the range from 0% to 100%.

- **Telemetery data**:
  collection of measurements or other data
  from a simulated vehicle on the host computer.

- **UI**: User Interface.
