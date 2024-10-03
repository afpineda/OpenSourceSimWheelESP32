# Integration test strategy

Ensure that all unit test have failed before proceeding to integration testing (*YES, they have to fail, since their purpose is to reveal bugs, but they don't*).

- This is an incremental, bottom-up strategy.
- The `capabilities` module is always integrated but not explicitly shown below.
- Just the modules being integrated are shown.

Run order:

```mermaid
flowchart TD
    subgraph Path 1
        TelemetryIntegrationTest --> Proto1
        DigitalInputsTest --> InputHubTest --> Proto1 --> Proto2
        BatteryAutocal --> Proto2
    end
    subgraph Path 2
        DigitalInputsTest2 --> I2CExpanderDeepSleepTest
    end
    TQTSystemTest
```

Render this graph at [mermaid.live](https://mermaid.live/view#pako:eNp9kE1vwjAMhv9K5DMc6LEHJLYijRuoOeZiGkMj5UupI6gQ_33pKsHGpvlg2a8fW698gy5oghpONly6HhML2SgvSgz5eE4Ye7FH7sVqFqeQZMkRp3HnmQrCJnhJA4vlci32KXD4BjfmbBjtzsfMw4P6aj_y8WXtWVbPC2_ITGncZA4d2l8Ief2X3-ofC9XsoXrfXiN6Takhiq0taZq-nJUH2Y4Dk5tmsABHyaHR5WW3CVDAffmGgrqUmk6YLStQ_l5QLJbb0XdQc8q0gBw1MjUGi0_3U9xqwyHN2v0TZvqDxA)

## *Test name*: [DigitalInputsTest](./DigitalInputsTest/README.md)

- DigitalPolledInput
  - RotaryEncoderInput
  - ButtonMatrixInput
  - AnalogMultiplexerInput
- inputs

## *Test name*: [DigitalInputsTest2](./DigitalInputsTest2/README.md)

- DigitalPolledInput
  - ShiftRegistersInput
  - I2CExpanderInput
- inputs

*Note:* This test differs from the previous one in the involved hardware.

## *Test name*: [I2CExpanderDeepSleepTest](./I2CExpanderDeepSleepTest/README.md)

- DigitalPolledInput
  - ShiftRegistersInput
  - I2CExpanderInput
- inputs
- **power**

*Note:* This test ensures the *inputs* subsystem properly configures interrupt pins at the GPIO expanders for wake up.

## *Test name*: [InputHubTest](./InputHubTest/README.md)

- RotaryEncoderInput
- DigitalPolledInput
- inputs
- **AnalogPolledInput**
- **userSettings**
- **inputHub**

## *Test name*: [TelemetryIntegrationTest](./TelemetryIntegrationTest/README.md)

- **hidImplementation**
- **notify**

*Note:* This test requires SimHub.

## *Test name*: [Proto1](./Proto1/README.md)

- RotaryEncoderInput
- DigitalPolledInput
- AnalogPolledInput
- inputs
- userSettings
- inputHub
- **hidImplementation**
- **notify**

*Note*: This is a working prototype

## *Test name*: [BatteryAutocal](./BatteryAutocal/README.md)

- power
- batteryCalibration

*Note*: Only autocalibration algorithm is tested.

## *Test name*: [Proto2](./Proto2/README.md)

- RotaryEncoderInput
- DigitalPolledInput
- AnalogPolledInput
- inputs
- userSettings
- inputHub
- hidImplementation
- notify
- **power**
- **batteryCalibration**

*Note*: This is a system test except for the involved hardware.

## *Test name*: [TQTSystemTest](./TQTSystemTest/README.md)

This is a *big-bang* test for the [LilyGO T-QT devkit board](https://github.com/Xinyuan-LilyGO/T-QT).
