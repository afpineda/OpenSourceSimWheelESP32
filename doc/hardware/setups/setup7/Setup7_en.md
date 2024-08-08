# Ready to deploy design #7

Read this document from start to end before building anything. Ensure you understand everything.
This setup was tested (with an external power source only).

## Hardware features

- Bluetooth Low Energy
- Powered through rechargeable batteries **or** external power source (but not both)
- Built-in battery monitor
- Analog clutch paddles (x2)
- Shift paddles (x2)
- Relative rotary encoders (with push button): x3
- Up to 20 additional push buttons

## Button mapping

- *Bite point calibration*: rotary #1 clockwise and counter-clockwise (while holding one and only one clutch paddle).
- *Next clutch function*: `Left shift paddle` and `START`.
- *Calibrate analog clutch paddles*: `Left shift paddle`,  `Right shift paddle` and `START`.
- *Power on*: rotary #1 clockwise or counter-clockwise.

## Needed parts

Common:

| **Item**                                     | **Quantity** | Notes                                                               |
| -------------------------------------------- | :----------: | ------------------------------------------------------------------- |
| Bare bone rotary encoder                     |      3       | With built-in push button                                           |
| Roller lever                                 |      2       | For shift paddles (maybe they are included with your wheel's case)  |
| Linear potentiometer (10K-ohms to 100K-ohms) |      2       | For clutch paddles (maybe they are included with your wheel's case) |
| Push buttons                                 |   up to 20   | General purpose inputs (up to you)                                  |
| PISO shift register                          |      3       | 74HC165N (*mandatory*)                                              |
| Resistors                                    |      25      | Any impedance in the range from 1K-ohms to 10K-ohms                 |
| Standard perfboard sized 24x10 holes         |      1       | Single-sided or double-sided                                        |
| LilyGO T-QT PRO DevKit board                 |      1       | With male pins already soldered                                     |
| Pin header (male or female up to you)        |      37      | For external wiring                                                 |
| External Antenna with proper connector       |      1       | Optional                                                            |

Battery-operated:

| **Item**                                      |    **Quantity**     | Notes                                                    |
| --------------------------------------------- | :-----------------: | -------------------------------------------------------- |
| LiPo Battery 4.2V (max)                       |          1          | Must fit T-QT specification. Capacity and size up to you |
| Both male and female GX16 (5 pins) connectors | 1 male and 1 female | For the charging port                                    |
| USB-C spiral cable long enough                |          1          | For charging. Must match your T-QT USB connector.        |
| Battery connector                             |          1          | Should be included in your T-QT purchase                 |

External power source:

| **Item**           | **Quantity** | Notes                                 |
| ------------------ | :----------: | ------------------------------------- |
| External connector |      1       | Depends on your external power source |

Other parts (quantity unknown):

- Wire with Dupond terminals (for external wiring). A kit for protoboards will do the job. ¿Male or female? the opposite to pin headers. Sacrifice some of them for other wiring.
- Welding tin.

Other notes:

- The GX16 connector may be replaced by any other kind. The GX16 features a knot that prevents accidental unplug. The 5 pins flavor allows to build a fully capable USB port, not just a charging/power port, if you want.
- The packaging of the shift registers will show a small circle next to pin #1, which is tagged as `~LD` here.
- There are several flavors of the T-QT DevKit board. Choose the **"PRO"** flavor, since it features a built-in powerboost module, unless you don't need the battery.
- The T-QT DevKit board is sold with some wires included for the (optional) battery.

## Pin-out plan for the T-QT DevKit board

| **GPIO** | **Input** | **Output** | **Usage** | **Notes**                  |
| -------- | --------- | ---------- | :-------: | -------------------------- |
| **34**   | OK        | OK         |  SERIAL   |                            |
| **33**   | OK        | OK         |  ROT1_A   |                            |
| **39**   | OK        | OK         |  ROT1_B   |                            |
| **38**   | OK        | OK         |  ROT2_A   | outputs PWM signal at boot |
| **37**   | OK        | OK         |  ROT2_B   | outputs PWM signal at boot |
| **36**   | OK        | OK         |  ROT3_A   |                            |
| **35**   | OK        | OK         |  ROT3_B   |                            |
| **48**   | OK        | OK         |   LOAD    |                            |
| **18**   | OK        | OK         |   NEXT    |                            |
| **17**   | OK        | OK         | POT2_OUT  | outputs PWM signal at boot |
| **16**   | OK        | OK         | POT1_OUT  |                            |

## Circuit layout

We are reusing the [shift registers example](../../subsystems/Switches/ShiftRegister.diy) from the [switches](../../subsystems/Switches/Switches_en.md) subsystem
as a satellite circuit:

![shift registers satellite circuit](../../subsystems/Switches/ShiftRegister.png)

[External wiring](./Setup7.diy) goes as follows:

![External wiring image](./Setup7.png)

This layout includes the following subsystems (read for an in-depth explanation):

- [Power](../../subsystems/Power/Power_en.md) through a powerboost module already built in the DevKit board or an external power source.
- [Battery monitor](../../subsystems/BatteryMonitor/BatteryMonitor_en.md) through a built in voltage divider.
- [Switches](../../subsystems/Switches/Switches_en.md) through shift registers.
- [Relative rotary encoder](../../subsystems/RelativeRotaryEncoder/RelativeRotaryEncoder_en.md) bare bone type.

Notes and build tips:

- Some components may look very small, not matching their actual size. This is not a mistake. They must be placed in vertical position, so they lie in a minimal surface of the perfboard.

### External wiring

- Each input has an assigned number in the circuit layout. Certain inputs have a particular function, so attach them properly.
- The `POTn_GND` and `POTn_VCC` terminals of each potentiometer are interchangeable.
  If the clutch (or axis) goes to 100% when idle, swap those terminals.
  If you don't have the chance to swap those terminals,
  the companion app has the ability to swap axis polarity by user request.
- Push buttons:
  - There are two `COM` pins. They are intended for the left and right sides of the steering wheel, but you may use any of them.
  - Wire `COM` to just one terminal of each push button *in a chain*.
  - Wire the other terminal of each push button to one of the light-blue pins.
  - Bare bone rotary encoders: their built-in push button must be wired like any other push button, being `SW` and `SW COM` the involved terminals.
- In a battery-operated setup:
  - Solder the battery connector to the small solder pads on the back of the T-QT devkit board.
  - `Battery(+)` is the positive pole of the battery.
  - `Battery(-)` is the negative pole of the battery.
  - Attach `Battery(+)` and `Battery(-)` properly. **Take care not to swap those terminals**. There are two small solder pads on the back of the T-QT for them.
  - `EXT_GND` is used as an additional `COM` pin header.
  - Battery charge port:
    - Cut the USB cable near the USB-C end, but leave enough length to reach the GX16 plug.
    - Make sure matching GX16 pins in both male and female connectors are soldered to the same wire (same color).
    - Red and black wires are mandatory. Others are optional.
- In an externally powered setup:
  - Solder the external connector to Dupond wires matching the pin header of `EXT_5V` and `EXT_GND`.
  - Leave `Battery(+)` and `Battery(-)` unwired (obviously).
  - `EXT_5V` is the positive terminal of the external power source. Any DC source from +5V to +7V is suitable.
  - `EXT_GND` is the negative (or ground) terminal of the external power source.
- Rotary encoder #1 is mandatory as well as the *Start* button.

## Firmware upload

At Arduino IDE, configure the board manager as this:

- Board: "ESP32S3Dev"
- USB CDC on boot: "Enabled".

You may need to edit the [sketch](../../../../src/Firmware/Setup7/Setup7.ino).

In a battery-operated setup:

- **You should calibrate your battery first**. See the [Battery calibration procedure](../../../../src/Firmware/BatteryTools/BatteryCalibration/README.md).
- Locate the `#define BATTERY_ENABLE_READ_GPIO` directive and rewrite, so it looks as follow:

  ```c
  #define BATTERY_ENABLE_READ_GPIO GPIO_NUM_4
  ```

When using an external power source:

- Locate the `#define BATTERY_ENABLE_READ_GPIO` directive and rewrite, so it looks as follow:

  ```c
  // #define BATTERY_ENABLE_READ_GPIO GPIO_NUM_4
  ```

1. Plug the USB cable to the DevKit board and upload the [sketch](../../../../src/Firmware/Setup7/Setup7.ino) with Arduino IDE. If required, in order to enter "boot mode" click the reset button while holding "IO0".
2. Open the serial monitor (Arduino IDE).
3. Reset.
4. Check there are no error messages.
