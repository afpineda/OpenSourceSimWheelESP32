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
