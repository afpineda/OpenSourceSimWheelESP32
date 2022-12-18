## Firmware (software) architecture

### Modules

The _system_ have been broken into several _modules_ that have been implemented as C++ namespaces. All of them are defined at _SimWheel.h_:

- **capabilities**: Everything realated to the capabilities of the hardware and firmware.
- **language**: Everything related to the language of the user interface.
- **inputs**: Everything related to hardware inputs and their events.
- **inputHub**: Everything related to the combined state of all inputs and their treatment. Translates input events into HID messages. Implements the behavior of clutch paddles, bite point calibration and "ALT" buttons.
- **uiManager**: Everything related to the coordinated use of the UI display from multiple modules.
- **ui**: Everything related to the user interface.
- **configMenu**: Everything related to the integrated menu used to set user preferences.
- **hidImplementation**: Everything related to the HID and NuS protocols.
- **power**: Everything related to power management.
- **uartServer**: Everything related to AT command processing and data capture for display.
- **batteryCalibration**: Everything related to estimation of battery charge.

Each namespace is implemented in a single _cpp_ file with its name, however, some of them have alternate implementations in order to enable unit and integration testing. Those files are named following this pattern: `<namespace><underscore><implementation>.cpp`. Some implementations are:

- _mock_: dummy implementation with no actual behavior.
- _serial_: implementation providing output to the serial port instead of the OLED.

All modules can be found at the `/common` folder.

### Auxiliary modules

Some namespaces are implemented with auxiliary modules which are not exposed at _SimWheel.h_, one _cpp_ file for each:

- _RotaryEncoderInput.cpp_: Everything related to rotary encoders
- _ButtonMatrixInput.cpp_: Everything related to button/switch matrices
- _debugUtils.cpp_: Minor utilities for debugging and testing
- _PolledInput_: Related to inputs that must be read in a polling (or sampling) loop.

### Module dependencies

Only most relevant methods are shown:

```mermaid
classDiagram
    class RotaryEncoderInput 
    class PolledInput
    class inputs { 
      +notifyInputEvent()
      +start()
    }
    class inputHub {
      +onStateChanged()
    }
    class language
    class ui {
      +clear()
      +display()
    }
    class uiManager {
      +enterDisplay()
      +exitDisplay()
    }
    class hidImplementation {
      +reportInput()
    }
    class configMenu {
      +onInput()
      +toggle()
    }
    class uartServer {
      +onReceive()
    }
    class power {
      +powerOff()
    }
    class batteryCalibration {
      +getBatteryLevel()
      +getBatteryLevelAutoCalibrated()
    }
    inputHub <-- inputs: events
    inputs <--> RotaryEncoderInput: events
    inputs <--> PolledInput: events
    inputs <--> ButtonMatrixInput: events
    inputHub --> ui: show bite point or save notifications
    inputHub --> configMenu: re-routed events
    inputHub <-- configMenu: user preferences
    inputHub --> hidImplementation: processed events
    ui --> uiManager
    configMenu --> language: set UI language
    configMenu --> ui: menu options
    hidImplementation --> ui: notify connection status 
    hidImplementation --> power: auto power-off 
    hidImplementation <-- power: current battery level
    hidImplementation <--> uartServer: NuS protocol
    uartServer --> inputHub: execute AT commands
    power --> ui: turn display off
    power --> batteryCalibration
```

[Render this graph at mermaid.live](https://mermaid.live/view#pako:eNp9VcGO2jAQ_RUrp1ZdfiCqVtpdkIpU2mppb1yMMwmWHE9kj1kQ4t87DgnrkACneOa9sef5jTllCgvI8kwZ6f1cy8rJemMF_9qIeEeS7riwEeaWtgkk0vQfNAaKNp6GdQx4ceqwQnyzSLo8tsDFHix9-XpNed7huj6PyvwIW3G6gtGuSRK87aStoJhkGU4FWUEaCzqpoQxIlxyg0L4x8jhZLOiVtFzMJXw-Prj5kBTDB03zB6V2uljWjYGa-ZI02qSkgwYdtfJMchXaUlcrsGGgxYDAEcKqMjDdCIu8BrcfdIL2HRTo_TSlwY8Bul3_LstJ8FYSq3J8k0Zv3W17FdDrJf8T9mCSE99kXgJhX2J0vVc7fJ_NOovlAqKbfALwMf08Ydz72MTF90GvgQjtSpLTh3vQeLiIDToXfocfYqsJWEdtSaATXu5BtJOgVSvRFPfzqnPhYOYwsBLTW0UdUnjwfF-NgxIcWAVT1UcmzJmAjPU3m_DEXDrp_N_d9acPY7YfNe4WSPxb3s7eEB1VqeMCm6T58Vj02MubEatYUG2GnwoKXjwitibNhWQjXb5nWJZ3GVHBjqGCY9WoN7Iw0Y8PeM_JSOXiV1hHIQkVdqRk4CK4vwV2zQEU36l4-cut1bW0RafEZd767ik4K7qnSXAPt5jxwEVE9pTV4GqpC37U2wHcZLTjk2-ynD8LKGUwtMk29szQ0BQ8Z4tCE7osJxfgKYvKrY9WZXkpjYce1P03dNHzf-lRDuA)

Many modules have a `begin()` method that must be called at system startup (`main()`or `setup()`). The calling order is defined by the previous diagram, where bottom modules must be called first.

### Definitions (header files)

All header files can be found at the `/include` folder.
Most relevant are:

- **SimWheel.h**: definition of all modules (namespaces).
- **SimWheelTypes.h**: common constants and types for all modules.
- **strings.h**: all message strings, already translated as requested by the user.
- **debugUtils.h**: constants and utilities for unit testing.

## Brief description of most relevant modules

For detailed description, see the doxigen's documentation at _SimWheel.h_.

### RotaryEncoderInput

Each detent of a rotary encoder generates two input events in quick succession: a button press and then, release. This requires a dedicated thread for every rotary encoder, but they are dormant most of the time.
Decoding is implemented by hardware interrupts.

### PolledInput and ButtonMatrixInput

There is a dedicated daemon that read the state of those inputs in a loop, every few milliseconds. Since many inputs are read at the same time, the combined state of all of them is reported to `inputHub`. Nothing is reported if there are no input event, this is, a state change since the previous iteration.

### Inputs

This is the place where inputs are set up and a number assigned to them. Use the `add*()` methods to tell the system input pins and kind of inputs (digital buttons, button matrices and rotary encoders). Input numbers are assigned by the system in calling order (ascending).

### InputHub

All the logic behind the behavior of the sim wheel is implemented at this module. Wheel's functions are mapped to input numbers here, using the `set*()` methods.

### Capabilities

This module holds static data about device capabilities. For example, it tells if the device has clutch paddles or not.
Such data is set from other modules at startup. This module is trivial, so it is not shown in the previous diagram. It may be called from any other module.

### BatteryCalibration

#### Accurate algorithm

Battery calibration is required for accurate battery levels. Calibration goes from full charge to battery depletion, taking a sample of battery voltage every minute. All possible voltages are divided into 32 evenly distributed ranges, called _quantums_. Calibration data is just a set of counters of voltage samples for each quantum. The sum of all counters is equivalent to 100% battery charge. Calibration data is stored in flash memory.

Let's be $V_{min}(i)$ the minimum voltage that falls into quantum $i$ (a natural number), $a < b \iff V_{min}(a)<V_{min}(b)$. Let's be $QSIZE = V_{min}(i+1)-V_{min}(i)+1$ (the same for all quantums). Let's be $S(i)$ the count of samples for quantum number $i$. Let's say we have a battery voltage $V_n$ that falls into the quantum number $n$ (0-index).

$BatteryLevel = \frac{ (\sum_{i=0}^{n-1}S(i)) + \frac{S(n)*(V_n-V_{min}(n))}{QSIZE} }{ \sum_{j=0}^{31}S(j) } * 100$

#### Auto-calibrated algorithm

If calibration data is not available, a rough estimation is provided based on LiPo batteries characterization data taken from here: [https://blog.ampow.com/lipo-voltage-chart/](https://blog.ampow.com/lipo-voltage-chart/). However, actual battery voltages may not match the characterization data due to 1) inaccurate ADC readings and 2) voltage drop due to the involved transistors. For this reason, the highest voltage ever read is taken as an auto-calibration parameter. The expected voltage reading is mapped linearly to the absolute maximum voltage ever read. The battery needs a full charge before this algorithm provides any meaningful result.

(See [LiPoBatteryCharacterization.ods](./LiPoBatteryCharacterization.ods))

## About inputs and input events

Every input is assigned a single number starting from 0 and up to 63.

The state of an input is represented by a single bit, 1 meaning a pressed button, 0 meaning a released button. So, the combined state of all inputs is represented as a 64-bits word, where the n-th bit represents the n-th input number. This is called an _input bitmap_. Least significant bit is numbered as zero.
For example, the word `00000000 ... 00000101` means that buttons 0 and 2 are pressed, and the others are released.

An input can also be identified by a bitmap. For example, the button number 3 can be expressed as `...01000`.

### Input masks

A mask is a 64-bits word where each bit represent a button number (the same as input bitmaps), but a bit set to 1 means that an input is not set and _must be ignored_ in the corresponding bitmap. Input masks are used in combination with bitmaps to build a combined state. For example:

| State              | Bitmap   | Bitmask  |
| ------------------ | -------- | -------- |
| (A) previous state | 11000001 | 11111111 |
| (B) partial state  | 00000010 | 11111100 |
| (C) new state      | 11000010 | 11111111 |

where `bitmap(C) = bitmap(B) OR (bitmask(B) AND bitmap(A))` being AND/OR bitwise operators.

### Event processing

Input events are captured from hardware interrupts or daemons. Most relevant are:

- **Rotary encoder daemons**: simulates a button press on each "detent". These daemons are dormant most time. They react to hardware interrupts.
- **Input poller daemon**: it checks the state of polled inputs every 50 ms. This period is short enough not to miss any event, but long enough to prevent other threads from starvation.

Event processing takes long, so later input events would be missed while processing sooner ones. To prevent this, input events are posted into a queue.

```mermaid
flowchart LR
  RE((Rotary encoder daemon))
  IP((Input poller daemon))
  Q[input event queue]
  IH((Input hub daemon))
  RE -- event --> Q
  IP -- event --> Q
  Q -- event --> IH
```

[Render this graph at mermaid.live](https://mermaid.live/view#pako:eNptj8sOgkAMRX-l6QoS-QEWriSRxIXg0nFRmSokzAxOOhpi_Hd5yELjrj333DR9YuU0Y4qX1j2qmrzArlQWoMyiqHRCvge2o-NBExtn43iM830U5bYLAp1r25-wODZTxHe2ArfAgU9TabuU6nD-apQZJMnHT5I1FPONP7D4ZvkWV2jYG2r08MVzVBRKzYYVpsOo-UKhFYXKvgY1dJqEM92I85iKD7xCCuIOva2WfXY2DV09mRm-3lPKX_4)

Event capture is detached from event processing at the **input hub daemon**, which runs most of the code. Note that such a daemon is implemented inside `inputs.cpp`, not `inputHub.cpp`.

## About OLED

The system is a multi-threaded one, so access to OLED hardware must be synchronized in some way. This is the main purpose of `uiManager`, but there is more. Some messages are more relevant than others, so they are assigned a priority. Lower priority messages never get on top of higher priority messages. If a message is hidden, the next one with lower priority is displayed again.

```mermaid
sequenceDiagram
    ui->>uiManager: enterDisplay(priority)
    uiManager-->>ui: buffer
    ui->>ui: build screen into `buffer`
    ui->>uiManager: exitDisplay(priority)
    alt current screen has higher priority
        uiManager->>uiManager: mark given priority as visible
    else current screen has lower priority
        uiManager->>uiManager: display an update current priority
    end
    ui->>uiManager: hide(priority)
    alt matches current priority
        uiManager->>uiManager: clear display
        uiManager->>uiManager: find next visible priority, if any, and display. 
    else otherwise
        uiManager->>uiManager: mark given priority as non-visible
    end
```

[Render this graph at mermaid.live](https://mermaid.live/view#pako:eNqdkz1uwzAMha9CaGqApAfwkCkdO3X1EMaibaIylVJSfhDk7lWquK2BpEXrxTbx8T3q0T6ZxlsylQn0lkgaWjF2ikMtkK_Ei-Uy8TMKdqQVkETSFYetw-PDVtkrx-NsZK_Y4qOngk1qW9KJ0KXIzkJolEiAJXpYF259x_HA8Y4hughNUs1DjYI9Bui560lhhAs7HXDiMKC-Qse73D72QJbZceCNo9JOLtAtL-f3f7Gy5SCAAmlrMX5pTiVI7O00erZ0K4YBY9NTuCP3w0SNI9Rxrl_plsWC0CGO6XwazYHbfKx8x4xc9R7hW3o-5q3sOdB_FyJeFtOliDVzM5AOyDZ_wadLuTbZZ6DaVPnRUovJxdrUcs5oyfzJcvRqqqiJ5gZT9C9Hacb3wlx_glI8vwMYlxTu)

## About auto power off

When there is no bluetooth connection, the systems goes to advertising. If no connection is made in a certain time lapse, the system goes to deep sleep or power off.
