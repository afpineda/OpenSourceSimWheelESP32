# Index

## Build manual

- [Overview](./overview_en.md)
- [Required skills and tools](./skills_en.md)
- [Prepare the source code for compilation and uploading](./firmware/sourcesSetup_en.md)

## Electronics

- [Know your ESP32 DevKit board](./hardware/DevKits_en.md)

- [Overview of input hardware suitable (or not) for a sim wheel/button box](./hardware/InputHW_en.md)

- Ready to deploy designs:

| Name                                            | Usage          | DevKit             | Inputs summary                                                                              | Power source        | OLED | Other |
|:-----------------------------------------------:|:--------------:|:------------------:|:-------------------------------------------------------------------------------------------:|:-------------------:| ---- | ----- |
| [Setup1](./hardware/setups/setup1/Setup1_en.md) | Steering wheel | ESP32-WROOM-32E/UE | Rotary encoders x4, DPAD or funky switch (optional), potentiometer support, up to 24 inputs | Battery or external | yes  | BLE   |
| [Setup2](./hardware/setups/setup2/Setup2_en.md) | Button box     | ESP32-WROOM-32E/UE | Rotary encoders x4, up to 52 inputs                                                         | external            | no   | BLE   |
| [Setup3](./hardware/setups/setup3/Setup3_en.md) | Steering wheel | ESP32-WROOM-32E/UE | Rotary encoders x4, DPAD or funky switch (optional), potentiometer support, up to 24 inputs | Battery or external | yes  | BLE   |

- [How to customize in order to build your own hardware and firmware setup](./hardware/subsystems/CustomizeHowto_en.md).

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
- [Notes on HID device implementation](./firmware/HID_notes.md)
- [Change log](./changelog.md)