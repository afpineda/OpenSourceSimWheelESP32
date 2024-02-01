# Index

## Build manual and electronics

- [Overview](./overview_en.md)
- [Required skills and tools](./skills_en.md)
- [Prepare the source code for compilation and uploading](./firmware/sourcesSetup_en.md)
- [Overview of input hardware suitable (or not) for a sim wheel/button box](./hardware/InputHW_en.md)
- [Know your ESP32 DevKit board](./hardware/DevKits_en.md)
- [How to customize in order to build your own hardware and firmware setup](./hardware/subsystems/CustomizeHowto_en.md).

- Ready to deploy designs:

  |                      Name                       |     Usage      |       DevKit       |                                       Inputs summary                                        |  Main power source  |
  | :---------------------------------------------: | :------------: | :----------------: | :-----------------------------------------------------------------------------------------: | :-----------------: |
  | [Setup1](./hardware/setups/setup1/Setup1_en.md) | Steering wheel | ESP32-WROOM-32E/UE | Rotary encoders x4, DPAD or funky switch (optional), analog clutch paddles, 12 push buttons |      External       |
  | [Setup2](./hardware/setups/setup2/Setup2_en.md) |   Button box   | ESP32-WROOM-32E/UE |                             Rotary encoders x4, 52 push buttons                             |      External       |
  | [Setup3](./hardware/setups/setup3/Setup3_en.md) | Steering wheel |      TinyPico      |         Rotary encoders x2, DPAD (optional), analog clutch paddles, 10 push buttons         |       Battery       |
  | [Setup4](./hardware/setups/setup4/Setup4_en.md) | Steering wheel |      TinyPico      |                 Rotary encoders x3, analog clutch paddles, 10 push buttons                  | Battery or external |
  | [Setup5](./hardware/setups/setup5/Setup5_en.md) | Steering wheel |  LilyGO T-QT PRO   |                 Rotary encoders x3, digital clutch paddles, 9 push buttons                  | Battery or external |
  | [Setup6](./hardware/setups/setup6/Setup6_en.md) | Steering wheel |    Lolin32 Lite    |                 Rotary encoders x4, analog clutch paddles, 16 push buttons                  |      External       |
  | [Teabox](./hardware/setups/Teabox/Teabox_en.md) |   Button box   |    Lolin32 Lite    |                     Rotary encoder, two-way push lever, 3 push buttons                      |      External       |
  | [Setup7](./hardware/setups/setup7/Setup7_en.md) | Steering wheel |  LilyGO T-QT PRO   |                 Rotary encoders x3, analog clutch paddles, 20 push buttons                  | Battery or external |
  | [Setup8](./hardware/setups/setup8/Setup8_en.md) |   Button box   |  LilyGO T-QT PRO   |                             Rotary encoders x4, 21 push buttons                             |         USB         |

## Known issues

- [ESP32C3](https://www.espressif.com/en/products/socs/esp32-c3) boards exposes a slightly different power management API than other ESP32 boards. The firmware has been updated, but not tested. Deep sleep mode may not work properly in battery-operated setups on these boards.

## Contributing

- Ways to contribute:

  - Report bugs.
  - Contribute with an open design for a steering wheel housing.
  - Contribute a ready-to-deploy circuit layout for other ESP32-based boards.
  - Share your experience and contribute with a tutorial.

- Pull request policy (contributing with new code):

  - Code for custom projects will be rejected. Provide code that everyone can benefit from.
  - All relevant quality controls must be updated along with the new code.
  - You must run all relevant quality controls.
  - Provide an explicit declaration that all relevant quality controls failed (so, the software passed the Q controls).
  - You must contribute to the firmware documentation as you do with code.

## Firmware documentation

- [Glossary of terms and definitions](./firmware/glossary_en.md)
- [Firmware architecture](./firmware/FirmwareArchitecture_en.md)
- [Overview of Quality Controls](./firmware/FirmwareTesting_en.md)
- [Notes on HID device implementation](./firmware/HID_notes.md).
  If you plan to develop an application that interfaces this firmware, you should read this.
- [Change log](./changelog.md)

## Other

- [Companion app](https://github.com/afpineda/SimWheelESP32Config)
- [Some thoughts from the author](./Thoughts_en.md)
