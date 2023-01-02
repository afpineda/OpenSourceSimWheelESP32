# Integration test strategy

Ensure that all unit test have failed before proceeding to integration testing (*YES, they have to fail, since their purpose is to reveal bugs, but they don't*).

- This is an incremental, bottom-up strategy.
- The `capabilities` module is always integrated but not explicitly shown below.
- Just the modules being integrated are shown.

## Step

> *Test name*: [DigitalInputsTest](./DigitalInputsTest/README.md)

- RotaryEncoderInput
- DigitalPolledInput
- inputs

## Step

> *Test name*: [InputHubTest](./InputHubTest/README.md)

- RotaryEncoderInput
- DigitalPolledInput
- inputs
- **AnalogPolledInput**
- **clutchState**
- **inputHub**

## Step

> *Test name*: [Proto1](./Proto1/README.md)

- RotaryEncoderInput
- DigitalPolledInput
- AnalogPolledInput
- inputs
- clutchState
- inputHub
- **hidImplementation**
- **notify**

*Note*: This is a working prototype

## Step

> *Test name*: [BatteryAutocal](./BatteryAutocal/README.md)

- power
- batteryCalibration

*Note*: Only autocalibrated algorithm is tested.

## Step

> *Test name*: [Proto2](./Proto2/README.md)

- RotaryEncoderInput
- DigitalPolledInput
- AnalogPolledInput
- inputs
- clutchState
- inputHub
- hidImplementation
- notify
- **power**
- **batteryCalibration**

*Note*: This is a system test except for the involved hardware.
