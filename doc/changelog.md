# Change log

## 1.0.0

First release.

## 1.1.0

- API changes in order to get more control over assigned input numbers, other than sequential numbering. See documentation for the `inputs` namespace, methods with suffix "Ext". More than one button matrix is allowed this way.
- Fixed wrong behavior of ALPS RKJX series of rotary encoders (and potentially others).
- Minor documentation improvements and fixes.

## 1.1.1

- Fixed bug in `inputs::start()` that was causing a wrong call to `abort()` in some circumstances.

## 1.1.2

- Fixed minor documentation errata.

## 1.1.3

- Issue #1: fixed boot up problem with external power sources and a misleading tag in all hardware designs.

## 2.0.0

- Now, the device may be configured through HID feature reports from the hosting PC, so there is no need for the OLED menu. A companion application has been developed (source code not included in this project).
- OLED is not mandatory anymore. This means two more GPIO pins are available.
- Telemetry data has been removed since there is no PC application for that. However, it may resurrect in the future.
- NuS and AT commands are not needed anymore. Replaced by HID reports.
- Analog clutch paddles are now fully supported. They may be configured as independent analog axes, so they may be mapped to throttle and brake, for example, for the disabled.
- A simple voltage divider is now supported as a battery monitor. This kind of circuit is built into many ESP32 DevKits.
- A "notification" subsystem replaces the OLED-based user interface, which is not tied to a particular hardware. A frame server is available, too.
- Removed documentation in Spanish. A nightmare to maintain.
- The battery calibration/backup procedures have been simplified. Now, no particular circuit is required.

## 2.1.0

- Analog multiplexers are now supported as input circuitry, which simplifies everything, requires less GPIO pins and allows smaller DevKits and circuit boards. Call this "miniaturization".
- New "ready to deploy" designs.
- Despite several ESP32 DevKit boards were always supported, now explicit advise is given on the matter.
- Improved documentation and minor errata fixes.

## 2.2.0

- Added "the teabox" design.
- Minor errata fixes.

## 2.2.1

- Minor errata fixes.

## 2.3.0

- PISO shift registers are now supported as input circuitry.
- Fixed wrong "includes.txt" files in some setups.
- Minor errata fixes and documentation updates.
- Added another "ready to deploy" design based on shift registers.

## 2.3.1

- Fixed some documentation errata.

## 2.3.2

- There are no changes in the source code of this version.
- Added a project logo.

## 2.3.3

- Issue #2: fixed compilation errors on ESP32C3 boards due to a different power management API.
  The firmware has been updated to work with these boards. However, the fix cannot be tested since I don't have such a board.
- Minor errata fixes on documentation.

## 2.3.4

- Improved documentation.
- Added test unit for deep sleep on ESP32C3 boards (but not run).
- Improved BLE advertising.
- Added USB implementation (for USB-capable boards like the ESP32S3).
- Added another "ready to deploy" design as an example of USB implementation.

## 2.3.5

- Fixed some documentation errata.

## 2.4.0

- Added BLE implementation through the "native" ESP-Arduino stack. This is a caution against bugs.
  NimBLE continues to be the default stack.
- The "Teabox" design now uses that implementation. Button mapping has changed, too.

## 2.4.1

- The "Teabox" design has been reverted to the NimBLE stack due to issue #3.

## 2.4.2

- Due to issue #4, explicit advice is given about the wiring of funky switches. No changes in the source code.

## 3.0.0

- Huge source code rework and refactoring. Technical debt paid.
- Rework on rotary encoders: no dedicated task is required anymore, thus saving a significant amount of memory.
- Test unit for deep sleep on ESP32C3 boards is not needed anymore.
  [DeepSleepTest](../src/QualityControls/UnitTests/DeepSleepTest/README.md) do the work for all boards.
- Unified unit test procedure for all HID implementations.
- DPAD inputs may work both as navigation controls or regular buttons depending on user preferences.
- The user can choose which input number is reported for each physical button, even in alternate mode.
  To swap steering wheels is much easier, since you can make your open source sim wheel to behave like any other sim wheel you have.
- HID implementation has been updated to support all the new features. Companion app will be updated soon (if not done yet).

## 3.0.1

- Minor changes to the "teabox" firmware to take advantage of new features.

## 3.0.2

- Moving to EUPL 1.2.
- No changes in the source code.

## 3.1.0

- Two brands of GPIO expanders (on the I2C bus) are now supported as input circuitry (for switches only)--almost
  unlimited switches with just two pins. This is the recommended way to go from now on.
- Another [integration test](../src/QualityControls/IntegrationTests/DigitalInputsTest2/README.md)
  is in place for GPIO expanders and PISO shift registers.
- Added another "ready to deploy" design as an example of GPIO expanders.

## 3.1.1

- Fixed a mistake in DIY files, affecting "Setup 9".
- No changes in the source code.

## 3.2.0

- The deep sleep API has been simplified.
  A single pin, active at low voltage, may be configured as a wake-up source.
  You don't want to spend more GPIO pins on this.
  Many signals could be combined into a single wake-up pin using diodes, if required.
- The firmware now enables GPIO expanders to be used as wake-up sources through the interrupt pin(s).
  Another integration test is in place for this use case.
- Fixed some documentation errata.

## 4.0.0

- In order to improve the "customization experience", the API now hides implementation details like switch *indices*.
  There is no need to deal with pointers, arrays, sizes, and lengths.
  Now, input numbers are directly assigned to their position in the hardware design.
  On the other side, you have to write more code.
- This is a "quality of life" improvement, but firmware functionality stays the same.
  A small [migration guide](./migrate_to_v4.md) has been added.
- All "ready to deploy" designs are updated to the new API, as well as the quality controls.

## 4.0.1

- Duplicated code has been removed from Setup9.ino
- Minor errata fixes.
- Help on error messages.

## 4.0.2

- Improved documentation and errata fixes.
- No changes in the source code.

## 4.1.0

- Code updated to work with [ESP-Arduino core v3.0.1](https://github.com/espressif/arduino-esp32/releases/tag/3.0.1).
  Integration tests passed.
- Minor errata fixes.

## 4.1.1

- Bug fix in [Setup9.ino](../src/Firmware/Setup9/Setup9.ino) due to wrong input numbers.

## 4.1.2

- Bug fix in [Setup3.ino](../src/Firmware/Setup9/Setup3.ino) due to wrong pin number at `BATTERY_READ_GPIO`.
- Minor documentation changes due to outdated content.

## 4.1.3

- More bug fixes at "Setup4.ino", "Setup5.ino" and "Setup7.ino".

## 4.1.4

- No code changes.
  [NimBLEImplTest](../src/QualityControls/UnitTests/NimBLEimplTest/README.md) passed with
  [NimBLE-Arduino v1.4.2](https://github.com/h2zero/NimBLE-Arduino/releases/tag/1.4.2).
- Minor errata fixes and documentation updates.

## 4.1.5

- No code changes.
- A new section has been added about how to rename the HID display name,
  addressing a [question](https://github.com/afpineda/OpenSourceSimWheelESP32/discussions/10)
  from user [@WallK](https://github.com/WallK).
- Major documentation changes due to errata and outdated content.

## 4.2.0

- A parameter was added to `hidImplementation::begin()` which allows to set a custom PID
  (BLE implementation only) on a device-by-device basis.
- Documentation updates.

## 4.3.0

- For security concerns, the API enables the user to lock or unlock configuration changes
  coming from any PC application (including the companion app).
- A custom VID and PID can be set using HID reports without the need to modify the source code
  (BLE only). The companion app supports this feature.
  In Windows, the companion app is able to change your device's display name, too.
- Documentation updates.

## 4.3.1

- The [Teabox](./hardware/setups/Teabox/Teabox_en.md) design now includes security lock functionality.
- There is a new specification for the "control code" field in HID Report 5.
- Errata fixes.

## 4.4.0

- New simple commands allow to change the polarity of any clutch paddle thanks to the companion app.
  Changes are saved to flash memory.

## 4.4.1

- Fix compilation bug in
  [BatteryCalibration.ino](../src/Firmware/BatteryTools/BatteryCalibration/BatteryCalibration.ino)

## 5.0

- The `power` namespace has been split into two namespaces (`power` and `batteryMonitor`) for maintenance reasons.
  Please, run the [source code setup procedure](./firmware/sourcesSetup_en.md) again.
  Minor changes are required in your custom firmware when using batteries.
- Added support for the "MAX1704x" family of "fuel gauges" in order to get better state of charge estimations (for batteries).
  They are connected to the I2C bus.
- Now the I2C bus works at higher speeds, depending on the capabilities of all connected chips.
- Bug fixes.
- ESP32S3-DevKit-C is the new default board for all quality controls.
- Errata fixes and documentation updates.
- More test units.
