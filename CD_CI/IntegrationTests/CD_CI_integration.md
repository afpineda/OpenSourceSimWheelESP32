# Integration strategy in the CD/CI chain

## Subsystems being integrated

The subsystem in **bold** text is the one making calls.
`[service]` means calls to the internal service class.
`[event]` means an internal event callback.

- StorateIntgTest

  - **storage**
  - [event] SaveSetting
  - [event] LoadSetting
  - All subsystems with user settings: [service]

- BatteryMonitorIntgTest

  - **batteryMonitor**
  - power: [service]
  - batteryCalibration: [service]
  - [event] OnLowBattery

- InputHubIntgTest

  - **inputHub**
  - map
  - hid
  - inputs: [service]
  - [event] OnBitePoint

- Report2Test

  - **hid**: `hid::common`
  - Class `DeviceCapabilities`

- Report3Test

  - **hid**: `hid::common`
  - inputs: [service]
  - inputHub: [service]
  - batteryCalibration: [service]
  - batteryMonitor: [service]
  - inputMap:[service]
  - pixels: `getCount()`
  - ui: [service]

- Report4Test

  - **hid**: `hid::common`
  - inputMap: [service]

- Report5Test

  - **hid**: `hid::common`
  - hid: [service]

- ReportTelemetryTest

  - **hid**: `hid::common`
  - telemetry

- ReportPixelControlTest

  - **hid**: `hid::common`
  - pixels

This set of testing units should cover all static and dynamic
internal dependencies from one caller subsystem to all called subsystems,
except those involving specific hardware.
