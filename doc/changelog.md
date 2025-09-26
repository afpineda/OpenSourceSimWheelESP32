# Change log

## 7.7.4

- Documentation updates.
- Now the CD/CI chain computes code coverage for automated tests (only).
- Fixed a bug in the LED strips caused by "pure" ESP32 boards not having DMA support.
  This was a blocking issue.
- Fixed a bug in the ADC readings caused by the ADC2 unit being shared
  with the Bluetooth radio (only on "pure" ESP32 boards).
  This was a blocking issue.
  This patch should improve the ADC performance a bit, too.
- Increased stack size for the input polling daemon.
  This was a blocking issue depending on the particular custom firmware.
- Unused code has been removed.
- A potential bug has been fixed regarding UI notifications.
  The task watchdog timer is reset between calls to event handlers.
- **Upgrading is highly recommended.**

## 7.7.3

- A custom DevKit PCB design has been added to the project,
  including customizable firmware and documentation.
- Performance tweaks for LED strips, including DMA mode and
  less memory consumption.
- Minor documentation updates.
- The *NimBLE-Arduino* dependency has been updated to version 2.3.5

## 7.7.2

- Two PCB modules have been tested and they work.
- Documentation on PCB modules has been updated accordingly.
- Minor documentation updates.
- No code changes except for an irrelevant header file.
  For this reason, there is no release for this version.

## 7.7.1

- The battery monitor subsystem has been reimplemented.
  This was done for extensibility, testability and testing.
- The firmware now detects whether the battery is charging
  so as not to report an inconsistent state of charge.
  Works with non-constant voltage battery chargers.
- It also detects whether the system is operating with wired power.
- Therefore, four parameters of the battery status are collected:
  charging status, battery presence, cable presence, and charge level.
  In the future, it may be possible for this information to be reported to the host computer.
- No changes are required in your custom firmware,
  but you must run the
  [sources setup procedure](./firmware/sourcesSetup_en.md) **again**.
- If available, firmware-defined error messages are reported
  in both the UART and USB serial interfaces.
- A new custom firmware sketch has been created for the upcoming custom PCB DevKit,
  which is still under development.
- Minor documentation updates.

## 7.7.0

- Added the missing implementation of `inputs::initializeI2C()`.
  Surprisingly, no sketch is using it so it went unnoticed.
- Now the internal pull-up resistors in the I2C bus can be disabled thanks to
  another `inputs::initializeI2C()` parameter.
- Additional advice and explanations are given regarding pull-up resistors in the I2C bus.

## 7.6.0

- Fixed inconsistent handling of pixel control notifications on startup and before connection.
- Battery presence is now available to the system in the `BatteryService` class.
- The `OnSettingsSaved` event is no longer triggered on the battery autocalibration setting,
  as that is not explicitly changed by the user.

## 7.5.0

- Pixel control notifications are prioritized over SimHub commands.
- Documentation updates regarding RGB LED strips without level shifter.
- Added a protected method to get the pixel count in `PixelControlNotification`.

## 7.4.3

- Fixed a bug in the pixel control feature which caused a system crash
  when using notifications.
  The bug only manifested itself in the low battery event.
  Thanks to Discord user *Darinho23* for reporting.

## 7.4.2

- A unique serial number has been added to the device information
  for all HID implementations.
  This is not currently useful, but it may be in the future.

## 7.4.1

- The battery level will now be shown briefly on startup
  in all user interfaces where available.
  Thanks to Discord user *Darinho23* for the suggestion.

## 7.4.0

- There is a new API that allows to wire
  rotary coded switches to any supported input hardware
  (not just the GPIO pins at the DevKit board).
- The API introduced in version 7.2.0 continues to work
  but is marked for deprecation.
- A bunch of PCB designs are introduced for
  auxiliary modules in the switches subsystem.
- Minor documentation updates and errata fixes.

## 7.3.1

- Fixes a bug in `inputs::add74HC165NChain()` reported in the Discord server.
  There are no integration tests regarding shift registers
  (there is just a unit test),
  so this API call is not tested in this project.

## 7.3.0

- In response to popular demand,
  a new parameter has been added to the `inputs::add74HC165NChain()` function
  that allows the switches to be configured to work in positive logic.

## 7.2.0

- A new notification is triggered whenever a user setting is saved to memory.
  The 'Pixel control' and 'PCF8574-driven rev lights' features
  implement a handler for this notification.
- Support has now been added for rotary coded switches with a BCD output.
- Minor documentation updates and errata fixes.

## 7.1.3

- External dependencies have been updated and tested:
  - Arduino-ESP32 support package: version **3.2.0**
  - NimBLE-Arduino: version **2.3.0**
- Development dependencies have been updated and tested:
  - Arduino IDE: version **2.3.6**
  - Arduino CLI (optional): version **1.2.2**
- Minor documentation changes.

## 7.1.2

- Button matrices configured for negative logic,
  now use open drain mode for the selector pins.
  This prevents a short circuit when the diodes are removed
  which is unfortunately a common use case among users.
  Note that **short circuits cannot be avoided** in a button matrix
  configured for positive logic without diodes.

## 7.1.1

- The code for active wait has been revised for even better accuracy.
- Documentation updates.

## 7.1.0

- The code for LED strips no longer require context switching to display all pixels.
  Active wait is used instead with nanosecond precision.
  This should improve performance.
- Active wait is also more accurate,
  improving performance a bit for button matrices,
  analog multiplexers and shift registers.
- Documentation updates.

## 7.0.2

- Important bug fixes affecting some "ready-to-deploy"
  designs based on ESP32S3 boards.
- Some "ready-to-deploy" hardware designs have changed:
  "Setup5" and "Setup7".

## 7.0.1

- Important bug fixes affecting some "ready-to-deploy"
  designs based on pure ESP32 boards.

## 7.0.0

- TL;DR
  - The code will check that you are not doing stupid things in your custom firmware.
  - You will have to rewrite your custom code, but the setup is more intuitive.
  - New features (thanks to @ArtTales for the suggestion):
    - *Virtual* neutral gear.
    - Configurable *default* input map.
  - Huge improvement in testability, extensibility and maintainability.

- This is a huge rewrite that affects the system architecture and most of the code base.
- The code will perform new checks to prevent a non-functional custom firmware, regarding:
  - Unusable, reserved, incapable, reused or non-existent GPIO pins.
  - Invalid input numbers.
  - Mapping non-existent hardware inputs to firmware features.
  - Incompatible firmware features.
- Error reporting no longer requires configuration in the Arduino IDE.
- Testability and extensibility improved with new architectural patterns:
  - Publish-Subscribe.
  - Dependency injection.
- Half of the code has been more or less rewritten to remove dependencies
  with embedded libraries. This has **deep consequences**:
  - This code can compile and run **in a PC workstation**,
    container or virtual machine.
  - Some test units are now automated, and do not require ESP32 hardware.
  - New hardware independent test units are available.
  - A **custom CD/CI chain** has been developed to take advantage of this.
    No external build tools are required other than the GNU C++ compiler.
- The old C-style code was rewritten in modern C++ style.
  User-defined code is more intuitive and expressive.
  Global variable declarations are no longer required
  (one less point of failure).
- A "virtual" button is available (optional).
  You can map a combination of hardware inputs to this "virtual" button.
  This is intended for shift paddles and **neutral gear**.
  You can map the engagement of both paddles to a neutral gear button.
- Now you can customize a *default* input map.
  This way, you are not forced to use the full range of 128 gamepad buttons
  if you only have a few physical buttons.
  This feature does not interfere with the ability to set a different input map
  using the *companion app*.
- Each UI instance runs on its own thread with its own stack size.
- The telemetry display using RGB LED strips has been **removed**,
  as pixel control is a better option.
- Analog multiplexers, button matrices and shift registers
  now actively wait for signal propagation with nanosecond precision
  and no context switching.
  This should improve the responsiveness of the switches.
- A [migration guide](./migrate_to_v7.md) is available.

## 6.11.4

- Fixed wrong queue size for rotary encoders.

## 6.11.3

- The HID specification for pixel control has been changed to
  remove the use of feature reports for `pixels::show()`.
  For unknown reasons, feature reports perform worse than output reports in Windows 11.
  For this to work, you will need to update the SimHub plugin as well.
- *BatchCompile.ps1* has been updated for a better look.

## 6.11.2

- Fix minor compiler error on ESP32-C3 boards.
- Battery capacity is explicitly advised.
- Other minor documentation updates.

## 6.11.1

- A very small but important bug in the *battery monitor* API has been fixed.
- Additional advice is given to avoid LED strip burnout.
  Thanks to @ArtTales for reporting this.
- Minor errata fix.

## 6.11.0

- For better host-side detection of rotation events,
  the behavior of rotary encoders has been improved.
- The default *pulse width* of the rotary encoders
  has been increased to 100 ms pressed and 100 ms released.
- The *pulse width* of all encoders can now be set as a time multiplier
  using HID reports (x1, x2, etc.).
- The new feature has been synchronously updated in the companion app.
- All relevant unit and integration tests have been updated.
- Minor documentation updates.

## 6.10.0

- "Launch control" is available as a new working mode for the clutch paddles.
- All relevant unit and integration tests have been updated.
- The new feature has been synchronously updated in the companion app and SimHub plugin.
- A new document explains the working modes of the clutch paddles.
- Minor bug fixes.
- Minor documentation updates.

## 6.9.1

- Code updated to work with the latest external dependencies:
  - NimBLE-Arduino version 2.2.0
  - Arduino-ESP3 version 3.1.1
- Documentation updates and errata fixes.

## 6.9.0

- Support for 16 and 32-channel analog multiplexers has been fixed
  by adding two new API calls: `inputs::addAnalogMultiplexer16()` and
  `inputs::addAnalogMultiplexer32()`.
- Documentation updates.

## 6.8.2

- Previous fix in 6.8.1 was incomplete.
  This patch should put an end to that issue.

## 6.8.1

- Fix Windows notifications not being sent on reconnection
  in the NimBLE-Arduino stack.
  The workaround is taken from the
  [ESP32-BLE-Gamepad](https://github.com/lemmingDev/ESP32-BLE-Gamepad/pull/257/files)
  project.

## 6.8.0

- The `inputs` namespace API will abort and display an error message
  if the custom firmware attempts to use a reserved GPIO pin (flash, psram, etc).
  This will be very helpful when troubleshooting boot problems.
- Documentation updates.

## 6.7.1

- Fix minor bugs.

## 6.7.0

- Data rate increased to 2Mbps for all BLE implementations (2M PHY).
- The default MTU (in BLE) has been adjusted to match the maximum HID report size.

## 6.6.0

- When the system shuts down,
  all display peripherals are also instructed to shut down.
  This was a missing feature.
- A new integration test has been added for this scenario.

## 6.5.0

- Pixel Control is now compatible with notifications (e.g., low battery).
  There are predefined notifications, but they are optional.
  Custom notifications can also be developed.
- Documentation is up to date with this.
  A workaround has been documented to make 5V LED strips work
  with 3.3V power sources.

## 6.4.3

- Small, but relevant, bug fix.

## 6.4.2

- The native BLE implementation (Arduino-ESP32) now works (v3.0.7).
  This is a caution against bugs and breaking changes.
- Code updated to work with
  [NimBLE-Arduino v2.1.0](https://github.com/h2zero/NimBLE-Arduino/releases/tag/2.1.0).
  NimBLE-Arduino continues to be the default BLE stack.
- Minor documentation updates.

## 6.4.1

- Documentation fixes and updates.
- Some code commentaries has been fixed for Doxygen to process them properly.
- No changes to the source code other than headers.
- The "build" GitHub action has been modified to only trigger
  on pushes to the "development" branch.
  This change does not apply to pull requests.

## 6.4.0

- "Pixel control" is available for LED strips.
  This is compatible with the "telemetry data" approach.
  The *SimHub plugin* has been updated to support this feature.
  Now, the end user can configure the behavior of each
  single pixel.
- Documentation updates.

## 6.3.1

- This changelog now lists updates in reverse order.
- Removed remaining compilation warnings,
  except for a single test sketch.
- Minor documentation updates.
- A build workflow using *GitHub Actions* has been implemented.
  This will compile all sketches under the "Firmware" path.
- There is also a batch build script that can be run on demand.
  This will compile all sketches under the "Firmware" and "QualityControls" paths,
  and displays any warnings.

## 6.3.0

- Support for more RGB LED drivers (but not tested).
- Four display modes are available to all "rev lights":
  left to right (default), right to left, center to edges and edges to center.
- Documentation updates.

## 6.2.1

- All warnings have been removed when compiling with Arduino IDE 2.3.2.
  Note that [C++ revision 20](https://en.cppreference.com/w/cpp/20)
  is the new standard.
- File `adcTools.cpp` has been rewritten to remove deprecated code.
- The I2C probe firmware has been improved for better information.
  You can now use the probe on the secondary I2C bus.
- Minor errata fixes and documentation updates.

## 6.2.0

- New telemetry display hardware: LED strips and single color LEDs.
- This display hardware includes "rev lights", gear shift lights, ECU witnesses
  and race flags.
- Support for single wire RGB LED strips based on WS2811, WS2812 and WS2812B pixel drivers.
- You can mix multiple telemetry displays in a single RGB LED strip.
- Integration tests are available for this display hardware.
- Documentation updated.

## 6.1.1

- Fix compilation bug using Arduino IDE 2.3.2 caused by the new C++20 compilation standards.

## 6.1.0

- A simple shift light is available as a telemetry indicator.

## 6.0.0

- Huge rework in the `notify` namespace.
  New implementation reduces memory footprint and CPU usage.
- Telemetry data is back thanks to HID reports.
  A [SimHub plugin](https://github.com/afpineda/SimWheelESP32-SimHub-Plugin)
  has been developed,
  but there is no display hardware yet.
  Wait for a future release or develop your own.
- The "Abstract Notifications Interface" is now called the "Abstract User Interface".
  It has been revised to handle telemetry data.
- The new API is not intended to implement full telemetry displays,
  as there are better choices for that.
  Intended uses are "rev lights", real car gauges, LED segments, etc.
- Many user interfaces can coexist, for example,
  *rev lights*, and a speaker.

## 5.0.0

- The `power` namespace has been split into two namespaces
  (`power` and `batteryMonitor`) for maintenance reasons.
  Please, run the [source code setup procedure](./firmware/sourcesSetup_en.md) again.
  Minor changes are required in your custom firmware when using batteries.
- Added support for the "MAX1704x" family of "fuel gauges" in order to
  get better state of charge estimations (for batteries).
  They are connected to the I2C bus.
- Now the I2C bus works at higher speeds, depending on the capabilities of all connected chips.
- Bug fixes.
- ESP32S3-DevKit-C is the new default board for all quality controls.
- Errata fixes and documentation updates.
- More test units.

## 4.4.1

- Fix compilation bug in
  [BatteryCalibration.ino](../src/Firmware/BatteryTools/BatteryCalibration/BatteryCalibration.ino)

## 4.4.0

- New simple commands allow to change the polarity of any clutch paddle thanks to the companion app.
  Changes are saved to flash memory.

## 4.3.1

- The [Teabox](./hardware/setups/Teabox/Teabox_en.md) design now includes security lock functionality.
- There is a new specification for the "control code" field in HID Report 5.
- Errata fixes.

## 4.3.0

- For security concerns, the API enables the user to lock or unlock configuration changes
  coming from any PC application (including the companion app).
- A custom VID and PID can be set using HID reports without the need to modify the source code
  (BLE only). The companion app supports this feature.
  In Windows, the companion app is able to change your device's display name, too.
- Documentation updates.

## 4.2.0

- A parameter was added to `hidImplementation::begin()` which allows to set a custom PID
  (BLE implementation only) on a device-by-device basis.
- Documentation updates.

## 4.1.5

- No code changes.
- A new section has been added about how to rename the HID display name,
  addressing a [question](https://github.com/afpineda/OpenSourceSimWheelESP32/discussions/10)
  from user [@WallK](https://github.com/WallK).
- Major documentation changes due to errata and outdated content.

## 4.1.4

- No code changes.
  [NimBLEImplTest](../src/QualityControl/UnitTests/HID/README.md) passed with
  [NimBLE-Arduino v1.4.2](https://github.com/h2zero/NimBLE-Arduino/releases/tag/1.4.2).
- Minor errata fixes and documentation updates.

## 4.1.3

- More bug fixes at "Setup4.ino", "Setup5.ino" and "Setup7.ino".

## 4.1.2

- Bug fix in [Setup3.ino](../src/Firmware/Setup3/Setup3.ino) due to wrong pin number at `BATTERY_READ_GPIO`.
- Minor documentation changes due to outdated content.

## 4.1.1

- Bug fix in [Setup9.ino](../src/Firmware/Setup9/Setup9.ino) due to wrong input numbers.

## 4.1.0

- Code updated to work with
  [ESP-Arduino core v3.0.1](https://github.com/espressif/arduino-esp32/releases/tag/3.0.1).
  Integration tests passed.
- Minor errata fixes.

## 4.0.2

- Improved documentation and errata fixes.
- No changes in the source code.

## 4.0.1

- Duplicated code has been removed from Setup9.ino
- Minor errata fixes.
- Help on error messages.

## 4.0.0

- In order to improve the "customization experience",
  the API now hides implementation details like switch *indices*.
  There is no need to deal with pointers, arrays, sizes, and lengths.
  Now, input numbers are directly assigned to their position in the hardware design.
  On the other side, you have to write more code.
- This is a "quality of life" improvement, but firmware functionality stays the same.
  A small [migration guide](./migrate_to_v4.md) has been added.
- All "ready to deploy" designs are updated to the new API, as well as the quality controls.

## 3.2.0

- The deep sleep API has been simplified.
  A single pin, active at low voltage, may be configured as a wake-up source.
  You don't want to spend more GPIO pins on this.
  Many signals could be combined into a single wake-up pin using diodes, if required.
- The firmware now enables GPIO expanders to be used as wake-up sources through the interrupt pin(s).
  Another integration test is in place for this use case.
- Fixed some documentation errata.

## 3.1.1

- Fixed a mistake in DIY files, affecting "Setup 9".
- No changes in the source code.

## 3.1.0

- Two brands of GPIO expanders (on the I2C bus) are now supported as input circuitry (for switches only)--almost
  unlimited switches with just two pins. This is the recommended way to go from now on.
- Another integration test
  is in place for GPIO expanders and PISO shift registers.
- Added another "ready to deploy" design as an example of GPIO expanders.

## 3.0.2

- Moving to EUPL 1.2.
- No changes in the source code.

## 3.0.1

- Minor changes to the "teabox" firmware to take advantage of new features.

## 3.0.0

- Huge source code rework and refactoring. Technical debt paid.
- Rework on rotary encoders: no dedicated task is required anymore,
  thus saving a significant amount of memory.
- Test unit for deep sleep on ESP32C3 boards is not needed anymore.
  [DeepSleepTest](../src/QualityControl/UnitTests/Power/DeepSleepTest/README.md) do the work for all boards.
- Unified unit test procedure for all HID implementations.
- DPAD inputs may work both as navigation controls or regular buttons depending on user preferences.
- The user can choose which input number is reported for each physical button, even in alternate mode.
  To swap steering wheels is much easier,
  since you can make your open source sim wheel to behave like any other sim wheel you have.
- HID implementation has been updated to support all the new features.
  Companion app will be updated soon (if not done yet).

## 2.4.2

- Due to issue #4, explicit advice is given about the wiring of funky switches. No changes in the source code.

## 2.4.1

- The "Teabox" design has been reverted to the NimBLE stack due to issue #3.

## 2.4.0

- Added BLE implementation through the "native" ESP-Arduino stack.
  This is a caution against bugs.
  NimBLE continues to be the default stack.
- The "Teabox" design now uses that implementation. Button mapping has changed, too.

## 2.3.5

- Fixed some documentation errata.

## 2.3.4

- Improved documentation.
- Added test unit for deep sleep on ESP32C3 boards (but not run).
- Improved BLE advertising.
- Added USB implementation (for USB-capable boards like the ESP32S3).
- Added another "ready to deploy" design as an example of USB implementation.

## 2.3.3

- Issue #2: fixed compilation errors on ESP32C3 boards due to a different power management API.
  The firmware has been updated to work with these boards.
  However, the fix cannot be tested since I don't have such a board.
- Minor errata fixes on documentation.

## 2.3.2

- There are no changes in the source code of this version.
- Added a project logo.

## 2.3.1

- Fixed some documentation errata.

## 2.3.0

- PISO shift registers are now supported as input circuitry.
- Fixed wrong "includes.txt" files in some setups.
- Minor errata fixes and documentation updates.
- Added another "ready to deploy" design based on shift registers.

## 2.2.1

- Minor errata fixes.

## 2.2.0

- Added "the teabox" design.
- Minor errata fixes.

## 2.1.0

- Analog multiplexers are now supported as input circuitry,
  which simplifies everything,
  requires less GPIO pins and allows smaller DevKits and circuit boards.
  Call this "miniaturization".
- New "ready to deploy" designs.
- Despite several ESP32 DevKit boards were always supported,
  now explicit advise is given on the matter.
- Improved documentation and minor errata fixes.

## 2.0.0

- Now, the device may be configured through HID feature reports from the hosting PC,
  so there is no need for the OLED menu.
  A companion application has been developed (source code not included in this project).
- OLED is not mandatory anymore. This means two more GPIO pins are available.
- Telemetry data has been removed since there is no PC application for that.
  However, it may resurrect in the future.
- NuS and AT commands are not needed anymore. Replaced by HID reports.
- Analog clutch paddles are now fully supported.
  They may be configured as independent analog axes,
  so they may be mapped to throttle and brake,
  for example, for the disabled.
- A simple voltage divider is now supported as a battery monitor.
  This kind of circuit is built into many ESP32 DevKits.
- A "notification" subsystem replaces the OLED-based user interface,
  which is not tied to a particular hardware. A frame server is available, too.
- Removed documentation in Spanish. A nightmare to maintain.
- The battery calibration/backup procedures have been simplified.
  Now, no particular circuit is required.

## 1.1.3

- Issue #1: fixed boot up problem with external power sources and a misleading tag in all hardware designs.

## 1.1.2

- Fixed minor documentation errata.

## 1.1.1

- Fixed bug in `inputs::start()` that was causing a wrong call to `abort()` in some circumstances.

## 1.1.0

- API changes in order to get more control over assigned input numbers,
  other than sequential numbering.
  See documentation for the `inputs` namespace, methods with suffix "Ext".
  More than one button matrix is allowed this way.
- Fixed wrong behavior of ALPS RKJX series of rotary encoders (and potentially others).
- Minor documentation improvements and fixes.

## 1.0.0

First release.
