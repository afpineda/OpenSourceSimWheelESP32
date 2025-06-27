# Index

> [!NOTE]
> Reading this documentation carefully from start to finish
> will save you a lot of time and effort.

## Build manual and electronics

- [Overview](./overview_en.md)
- [Required skills and tools](./skills_en.md)
- [Input hardware suitable (or not) for a sim wheel/button box](./hardware/InputHW_en.md)
- [Know your ESP32 DevKit board](./hardware/DevKits_en.md)
- [Prepare the source code for compilation and uploading](./firmware/sourcesSetup_en.md)
- [How to customize in order to build your own hardware and firmware setup](./hardware/subsystems/CustomizeHowto_en.md)
  - [Migrating your custom firmware to version 7 from a previous version](./migrate_to_v7.md)
- [Troubleshooting](./Troubleshooting_en.md)

- Ready to deploy designs:

  *These designs work as an example of several implementation choices. Setup9 is the recommended way to go.*

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
  | [Setup9](./hardware/setups/setup9/Setup9_en.md) | Steering wheel |   Wemos S3 mini    |      Rotary encoders x7, funky switch or DPAD, analog clutch paddles, 32 push buttons       |      External       |

- [How the clutch paddles work](./ClutchHowTo_en.md)
- [Changing your device's display name (Windows only) or Hardware ID (BLE only)](./RenameDeviceWin_en.md)

- Telemetry:

  - [Overview of telemetry](./telemetry_en.md)
  - [Working with LEDs and RGB LED strips](./LEDs_en.md)
  - [Firmware customization in the *pixel control* approach](./hardware/ui/PixelControl_en.md)
  - [Firmware customization in the *telemetry data* approach](./hardware/ui/TelemetryData_en.md)

- Printed circuit board (PCB) designs:

  - [Auxiliary module for 24 switches (through hole mounting)](./pcb/Module24sw_TH/Module24Switches_TH_2025.md)
  - [Auxiliary module for 32 switches (through hole mounting)](./pcb/Module32sw_TH/Module32Switches_TH.md)
  - [Auxiliary module for 32 switches (surface mounting)](./pcb/Module32sw_SM/Module32Switches_SM.md)

## Known issues

- [ESP32C3](https://www.espressif.com/en/products/socs/esp32-c3)
  boards exposes a slightly different power management API than other ESP32 boards.
  The firmware has been updated, but not tested.
  Deep sleep mode may not work properly in battery-operated setups on these boards.
- Device not working after a firmware upgrade. This issue is caused by an operating system cache. Fix:
  - BLE devices: remove from the Bluetooth control panel then pair again.
  - USB devices: remove from the device manager then reboot your computer.
    Ensure the operating system is detecting your device as new hardware.

## Contributing

**You may still contribute even if you don't have software development skills or in-depth knowledge of this project.**

See [how to contribute](../.github/CONTRIBUTING.md).

## Firmware documentation

- [Glossary of terms and definitions](./firmware/glossary_en.md)
- [Firmware architecture](./firmware/FirmwareArchitecture_en.md)
- [Overview of Quality Controls](./firmware/FirmwareTesting_en.md)
- [Notes on HID device implementation](./firmware/HID_notes.md).
  If you plan to develop an application that interfaces this firmware, you should read this.
- [API documentation](https://afpineda.github.io/OpenSourceSimWheelESP32/)
  (automatically generated by [Doxygen](https://doxygen.nl/))
- [Configuration management policy](./firmware/ConfigManagement_en.md)
- [Change log](./changelog.md)

## Other

- [Companion app](https://github.com/afpineda/SimWheelESP32Config)
- [SimHub plugin](https://github.com/afpineda/SimWheelESP32-SimHub-Plugin)
- [Some thoughts from the author](./Thoughts_en.md)
- Outdated migration guides:
  - [Migrating your custom firmware to version 4 from a previous version](./migrate_to_v4.md)
  - [Migrating your custom firmware to version 5 from a previous version](./migrate_to_v5.md)
