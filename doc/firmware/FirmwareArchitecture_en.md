# Firmware (software) architecture

The *system* has been broken down into several *subsystems*:

- `batteryCalibration`:
  Everything related to battery profiling.
- `batteryMonitor`:
  Everything related to the measurement of available battery charge.
- `firmware`:
  Performs initialization and launches execution.
- `hid`:
  Everything related to the HID protocol.
- `inputs`:
  Everything related to hardware inputs and their events.
- `inputHub`:
  Everything related to the combined state of all inputs and their treatment.
  Translates input events into a HID report.
- `inputMap`:
  Translates firmware-defined input numbers to user-defined input numbers.
- `pixels`:
  Everything related to pixel control.
- `power`:
  Everything related to power management.
- `storage`:
  Manages long-term storage of user settings in flash memory.
- `telemetry`:
  Holds received telemetry data (this subsystem is trivial).
- `ui`:
  Everything related to the user interface, if available.

Each subsystem is implemented through one or more code abstractions:

- **A "public" namespace**, matching the subsystem name.
  Contains code available for firmware customization.
- **An "internal" namespace**, matching the pattern `internals::<subsystem>`.
  Contains code for internal operation,
  not exposed for customization.
- **A "Service class"**, matching the pattern `<subsystem>Service`.
  Does not contain code, just an interface.
  A "service provider" descendant class
  is in charge of the actual implementation.
  For performance reasons, only non-critical code
  is placed in a service provider.
- Auxiliary classes and type names (do not belong to any namespace).
- Auxiliary namespaces which belong to `internals`, but do not name a subsystem:
  - `internals::hal`:
    Utilities for ESP32 hardware operation.

## Code structure

Those code abstractions are found in code artifacts:
header files (.hpp) and translation units (.cpp).
Some header files contain both a declaration and an implementation,
thus not requiring a translation unit.

- Header files:

  - Core:

    - `SimWheel.hpp`: declares all *public namespaces*.
    - `SimWheelTypes.hpp`:
      declares all type names required by public namespaces.
    - `SimWheelInternals.hpp`: declares all *internal namespaces*
      (except auxiliary)
    - `InternalTypes.hpp`:
      declares all type names required by internal namespaces.
    - `InternalServices.hpp`: declares all service classes.

  - Auxiliary:

    - `BatteryMonitorHardware.hpp`:
      required by the `batteryMonitor` subsystem.
    - `HAL.hpp`:
      declares the `internals::hal` namespace.
    - `HID_definitions.hpp`:
      required by the internal namespace `internals::hid`.
    - `InputSpecification.hpp`:
      required by the public namespace `inputs`.
    - `InputValidation.hpp` and `InputHardware.hpp`:
      required by the internal namespace `internals::inputs`.
    - `NimBLEWrapper.hpp`:
      custom wrapper to the underlying NimBLE API.
    - `OutputHardware.hpp`:
      required by the `pixels` subsystem and `SimWheelUI.hpp`.
    - `ServiceMocks.hpp`:
      Fake/mock *service classes* available for unit testing.
    - `SimWheelUI.hpp`:
      declares out-of-the-box user interfaces for telemetry display
      and notifications.
    - `Testing.hpp`:
      common utilities for Arduino-only test sketches.

- Translation units:

  - Core:

    Each subsystem is implemented in a *cpp* file with its name,
    including a public namespace, an internal namespace
    and a service class (if any).
    However, some of them have alternate implementations.
    Those files are named following this pattern:
    `<subsystem>_<implementation>.cpp`.

  - Auxiliary:

    Provide implementation for the matching header file.
    For example, `InputHardware.cpp` implements all declarations
    found in `InputHardware.hpp`.
    There is an exception:
    `SimWheelUI.cpp` and `SimWheelUI_gfx.cpp` share
    the implementation of `SimWheelUI.hpp`.

### Exceptions to this rule

- `hid::configure()` is implemented in `hidCommon.cpp`.
- The `HidService` service class is implemented in `hidCommon.cpp`.

## Principle of single responsibility

| Code artifact              | Reason to change                                                                          |
| -------------------------- | ----------------------------------------------------------------------------------------- |
| batteryCalibration.cpp     | SOC algorithm                                                                             |
| batteryMonitor.cpp         | Battery management capabilities                                                           |
| BatteryMonitorHardware.cpp | Battery management hardware design                                                        |
| firmware.cpp               | Firmware initialization                                                                   |
| HAL.cpp                    | Underlying ESP-IDF API                                                                    |
| hid_«implementation».cpp   | Underlying HID stack or wrapper                                                           |
| hidCommon.cpp              | Features available through the companion app or SimHub (see note)                         |
| InputHardware.cpp          | Input hardware design                                                                     |
| inputHub.cpp               | Device operation through specific button press combinations                               |
| inputMap.cpp               | Firmware-defined or user-defined input numbers                                            |
| inputs.cpp                 | Input hardware settings                                                                   |
| OutputHardware.cpp         | Output hardware design                                                                    |
| pixels.cpp                 | Pixel control capabilities or available UI notifications                                  |
| power.cpp                  | Underlying power management capabilities                                                  |
| SimWheelUI.cpp             | Out-of-the-box user interfaces for telemetry display and notifications not using graphics |
| SimWheelUI_gfx.cpp         | Out-of-the-box user interfaces for telemetry display and notifications using graphics     |
| storage.cpp                | User settings that require long-term storage                                              |
| telemetry.cpp              | Telemetry data                                                                            |
| ui.cpp                     | Support for custom user interfaces                                                        |

Note:

- Exceptionally, `hidCommon.cpp` includes common code related to
  BLE connectivity.
  This breaks the principle of single responsibility but is
  required to factorize code.

## Approach to dependency injection

Three kinds of dependency injections are found in this project:

- Static (dependencies are injected at compile time):

  A translation unit is replaced by another one implementing
  the same declarations.
  Thus, dependencies are injected at compile time in the build process.
  Static dependency injection does not provide dependency inversion,
  but is required for performance concerns.

- Dynamic (dependencies are injected at run time):

  - Service classes:

    They follow the dependency injection design pattern,
    but using static classes
    (not parameter injection nor constructor injection),
    since all of them require a singleton instance.
    This pattern involves virtual methods and there is a performance
    penalty in them. However, this pattern achieves dependency inversion.

    *Note*: Each service class is also a mock for itself.
    If no dependency is injected, the mock is automatically injected
    without a runtime failure. This simplifies testing.

  - Publish-subscribe events:

    An "event" class decouples the caller from the callee.
    The caller triggers the event (function call operator `()`) which,
    in turn, runs a number of subscribed callbacks unknown to the caller.
    This pattern removes static dependencies completely
    and there is almost no performance penalty.

  The procedure for dynamic dependency injection is this:

  1. The main program (an Arduino sketch file)
     calls `firmware::run()`.
  2. This method calls all `internals::<subsystem>::getReady()` methods.
  3. Each `getReady()` method subscribe to the required events and
     inject an instance for its service class.
     However, other dependencies are not injected yet,
     thus can not be called.
     For this reason, they typically subscribe to the `OnStart` event.
  4. `firmware::run()` triggers `OnStart()`.
  5. Each `OnStart` callback is executed.
     At this point, all dependencies are available to them.
     Each subsystem performs initialization and
     may call other subsystems if required.

## Internal dependencies

Only **most relevant information** is shown below.
A solid arrow means a static dependency.
A dotted arrow means a dynamic dependency through a service class.

### Core

```mermaid
classDiagram
    class inputs
    class AnalogInput
    class DigitalInput
    class inputMap
    class inputHub {
      +onRawInput()
    }
    class hid {
      +reportInput()
    }
    class ui
    class telemetry
    class pixels
    <<subsystem>> inputs
    <<subsystem>> inputHub
    <<subsystem>> hid
    <<subsystem>> telemetry
    <<subsystem>> pixels
    <<subsystem>> inputMap
    <<subsystem>> ui
    <<abstract>> AnalogInput
    <<abstract>> DigitalInput
    inputHub <-- inputs: input events
    ui <-- inputHub: routed input events
    inputs <-- DigitalInput: state of switches
    inputs <-- AnalogInput: axis position
    inputMap <-- inputHub : map input numbers (command)
    hid <-- inputHub: processed events
    pixels <-- hid: pixel data and commands
    telemetry <-- hid: raw telemetry data
```

[Render this diagram at mermaid.live](https://mermaid.live/view#pako:eNqFU8tu2zAQ_BWCpwR1DEl-qCaMAEVToD3k0t4KXdbS2iYgkQIftV3D_97Vww4VuYguFGdnlrNc7pnnukAueF6CtS8SdgaqTDH6WoRJVXtnQ-SLglLvfjR4CL_InXRQjvA2wSvUI-y737BzhzL2SaufcGjFD48deAkVe1kEZIO1Nu7_bC_DncMSK3TmFIK1PGLZ17VeW7-xJ-uwen4eVHwnQrbvxcjgPfjd2cPgBx5ulzaMXYtbr2FjnYHcEThqyiA67s2tA-unp75i0a0M_6C61u_lG4HYghntHRZ3mF2Olh2eJph14JDpLbMH6fI9jvmBd8HgKKk52kontQqodBkDK0ywiqDOiPLVBo1lD7muKlBF_yaaRzO0Xxudo7VUQei960NLJYno9qwAB4ySsT5pT7519I1v4BDAjY5PeIWmAlnQaLXvNuNuT4SMC_otcAu-dBnP1IWo4J3-dVI5F854nHBfUw7sh3EIfiuk0-aKUTd2ey62UFra1aB-a30TYEt97ce7WUiAqkDzVXvluJjHcSvi4syPXCTL6TxZJKtosUiXcRLFywk_cTFbTFdRGqereJbO0s_x6jLhf9tjommaEJKSaB4tV_OEBDvTlNwYuPwDL6JvAQ)

### Hardware inputs

Each box represents a C++ class.

```mermaid
classDiagram
    class I2CInput{
      #getGPIOstate()
    }
    class DigitalInput{
      +read()
    }
    DigitalInput <|-- ButtonMatrixInput
    DigitalInput <|-- AnalogMultiplexerInput
    DigitalInput <|-- ShiftRegistersInput
    DigitalInput <|-- RotaryEncoderInput
    DigitalInput <|-- DigitalButton
    DigitalInput <|-- I2CInput
    I2CInput     <|-- PCF8574ButtonsInput
    I2CInput     <|-- MCP23017ButtonsInput
    AnalogInput  <|-- AnalogClutchInput
```

[Render this diagram at mermaid.live](https://mermaid.live/view#pako:eNp9kl9PwjAUxb_Kcn3ROEnZBh2LLzrQ8EAk8Gb20mxlNBkt6W4TEPfdLR0Y0LC-tD39nf45twfIVcEhgbxidT0WrNRsk0nPNqd40yCdyq3BQyt63l3J8X0-_aiRIb9_aOXm0jIWpUBWXdseNWfFH_wS9J6_n568V4Oo5IyhFjsn3wJfJKtUOTMVim3Fd1x30su1WOGCl6JGrutOdKGQ6f1EHmPp3vSktFe-BZ3ja9fPM5eIW5-nb_GARu0mdSc6S-dBSPr0P9uGccIv4kkrg_na6eDDhusNE4WttStKBrjmG55BYocFXzGbZQaZbCzKDKrlXuaQoDbcB7MtbLFPv-NanBQClT5rWplyDcmKVbWdbZn8VOrXwB06O_23Y2cNXNqcU2UkQkKdBZID7CAZjnokosNR0A9pP4iHPuwhCWnQI3EQjcIBGUVRQBsfvtwRpBfTASEkIv0opjanyIdSH597PLz5AX3n7tA)

### Battery monitoring hardware

Each box represents a C++ class.

```mermaid
classDiagram
    class BatteryMonitorInterface{
      #getStatus()
    }
    class MAX1704x{
    }
    class VoltageDividerMonitor{
    }
    class BatteryStatus {

    }
    BatteryMonitorInterface .. BatteryStatus
    BatteryMonitorInterface <|-- MAX1704x
    BatteryMonitorInterface <|-- VoltageDividerMonitor
```

[Render this diagram at mermaid.live](https://mermaid.live/view#pako:eNqNkVFPgzAQx78KOV80YWQFRqXxRZ0PPuxpiTGGlwZujATapVzNJvLd7RhZtgSN7UPbu9-_92-vg1wXCALyWrbtspKlkU2mPDeGiPckidAcVlpVpM2rcoeNzLE7MZ53UyKtSZJtb-9Osf5Svnp8Z3we77uJ3JuuSZa4rD6rAs1YYQocPZzKeI64Yn5x6AXBtfBv-uF7Nju7_Qc6aR58aNA0sircjw4vyYC22GAGwm0L3EhbUwaZ6h0qLen1QeUgyFj0we4KSTj24Dr4UgzXjzGjbbkFsZF16047qT60PgtwQFdjV4-LE6ByJp-1VQRiMUhAdLAHEbEg5XMW8pAlfMGSJPLhACJmaRCx-5CncRhGbvY-fA1F5kEaRylf8NixLHFZH0pzfPCxfP8DeavBRg)

### Internal services

Each box represents a subsystem (not a class).

```mermaid
classDiagram
    class inputs {
    }
    class inputHub {
      +onRawInput()
    }
    class hid {
      +reportInput()
    }
    class batteryMonitor {
    }
    class batteryCalibration {
    }
    class power {
      +shutdown()
    }
    power <.. hid: shutdown on timeout (command)
    power <.. batteryMonitor: shutdown on critical battery level (command)
    inputs <.. inputHub: recalibrate axes (command)
    inputs <.. hid: recalibrate axes, set pulse width, reverse axes (commands)
    batteryCalibration <.. hid: restart auto-calibration algorithm (command)
    inputHub <.. hid  : working modes, bite point, etc
    inputMap <.. hid: current input map
```

[Render this diagram at mermaid.live](https://mermaid.live/view#pako:eNp9k7Fu3DAMhl-F0JSgbh7AyNYWaIdb0q3wwpOYs1BLFCgqThDcu1e-s1uf69aLberjz18U9W4sOzKtsQPm_NnjSTB0EepziYCPqWiG92vs_NfS13JcFgE-cHzC8dsUv7vfyei9W8FCiUX_TR9RleTtwNEry66FGfmEgz8Kque4iyUeSVaVc1_U8Rg3Za_Y48PDZLSFhYKqqj4QF4U7yyFgdPfbjFuzt8lWvHqLwwLBQC80bLXmTk9iS2dbELLz3gjwlfJ_ki6et3wDmRRSGTLB6J32TUVeSPJGLs96Ow1daWdFUcCi_NGuCBxOXLfYh11304TMGgAtjCw_fTxBqHNX7R19dZrYR22A1K7yDpj-1LZFhKJeVyBgMo0JJAG9q8N7OdnOaE-BOtPWT0fPWAbtTBfPFZ0sf3-L1rQqhRpTkqsdmsf9NvjFTQe4xITLqTftM9YGNiZh_MEcfv_ThT3MN2h6nX8BATIdXw)

## Event system

- Initialization and load/save settings:

  ```mermaid
  flowchart LR
    OnStart@{ shape: stadium }
    LoadSettings@{ shape: stadium }
    SaveSettings@{ shape: stadium }
    OnSettingsSaved@{ shape: stadium }
    any@{ shape: procs }
    firmware -- notify --> OnStart -- subscribed --> any
    any -- notify --> SaveSettings -- subscribed --> storage
    any -- notify --> LoadSettings -- subscribed --> storage
    storage -- notify --> OnSettingsSaved
    OnSettingsSaved -- subscribed --> ui
  ```

  [Render this diagram at mermaid.live](https://mermaid.live/view#pako:eNqFUslugzAQ_RVrziQiAprgQ1Wp7S1VpXKruDh4AEvBRl6aUsS_15ClaZsoPs3y3pvF00OhOAIFQsqt2hU105as33JJ_HuVmfX-Q09MzVqkxFjGhWvIsM-vFeMZWitkZa6CMvaBN0G-0gEywvlVHJPdT67VqjDHTCl0s2MayWxGpLKi7Lx1fxxhjBq3MYUWG-RTxkudNP-Qzlu-wDRWaVbhNfb5Vm6xD87_ps-3cXFFF5SdyCUE0KBumOD-T_uRmYOtscEcqDc5lsxtbQ65HDyUOauyThZArXYYgGs5s_gkWKVZ8zv4zIXv9RjTylU10JJtjfdaJt-VOhFwgr7s72o6L09AyVE_Kict0GSiAO3hE2iazqNlnMarJIpWYZSmAXRA4zCdJ-ndIknCZRQukjAeAviaioTz1dIrVHqccaw4fAPImedh)

- Battery level and shutdown:

  ```mermaid
  flowchart LR
    OnShutDown@{ shape: stadium }
    OnBatteryStatus@{ shape: stadium }
    OnLowBattery@{ shape: stadium }
    ui_pixels@{ shape: procs, label: "ui / pixels" }
    power -- notify --> OnShutDown
    batteryMonitor -- notify --> OnBatteryStatus
    batteryMonitor -- notify --> OnLowBattery
    OnShutDown -- subscribed --> ui_pixels
    OnBatteryStatus -- subscribed --> hid
    OnLowBattery -- subscribed --> ui_pixels
  ```

  [Render this diagram at mermaid.live](https://mermaid.live/view#pako:eNqNklFvgjAQx79Kc8_okIFoH5Zlc2-aJfNtIVkKLdIEWlLaICN89xWYTqcz61N79_vf9X9tC4mkDDCkuayTjCiN1m-RQHa9im1m9ErW4rFFVUZKhlGlCeWmQN0BeSJaM9VsNdGmusGtZf2N_gkZ_lHyPctPypRKJpWDchKzHKMIDEd3aIQiOOhKWTOFJhMkpOZpY3cPJ3cfmXjsvZGCa3kJn7n4l-LHz-9h9Whl4ipRPGZ0wI_Ork7tiiDj9HJwtwqDAwVTBeHUPmXbayPQGStYBP3cKEuJyXUEkegsSoyW20YkgLUyzAFTUqLZipOdIsV58IX29g8xJc0uA5ySvLKnkoh3KY8CNqCb8TsNv8oKmKBMPUsjNOBgkABuYQ94tpxP_fvlIghddx54YWizDWDPc6eu64ULG_SDMAg7Bz6HJu7Ud0Pfwq7NeaHvz3wHdqp33PfvvgBj1uoc)

  *OnShutDown* is a notification, not a command.
  However, the *ui* subsystem translates this event into a command
  for all user interface instances.
  To command a shutdown, the firmware needs to call
  `PowerService::call().shutdown()`.

- Connection/disconnection:

  ```mermaid
  flowchart LR
    OnConnected@{ shape: stadium }
    OnDisconnected@{ shape: stadium }
    ui_pixels@{ shape: procs, label: "ui / pixels" }
    hid -- notify --> OnConnected
    hid -- notify --> OnDisconnected
    OnConnected -- subscribed --> ui_pixels
    OnDisconnected -- subscribed --> ui_pixels
  ```

  [Render this diagram at mermaid.live](https://mermaid.live/view#pako:eNqFkctqw0AMRX9FaO3QvRehkHTXUmh2ZSDIM3IsmIeZB20w_vfaDm5S-tJKVxzBvdKAOhjGGlsb3nRHMcPji_Iw1bPfBe9ZZzb3A6SOeq4hZTJSHIwrs5ek_8WKHHt5Z5uuRB-DThVYatjWoLAI3MEFUrjudWJgswEfsrTnqdvemvodufX0LctMp9IkHaVZ1PZq76dQf_FYoePoSMx0wWHeVpg7dqxwzmS4pWKzQuXHCaWSw-HsNdY5Fq6w9IYy74VOkdzX4YORHOI6i6GcOqxbsmlSPfnXENyn5oV9urxx-eb4AeOSno0)

## Save settings

```mermaid
sequenceDiagram
  participant any
  participant storage
  participant timer
  any ->> storage: SaveSettings(userSetting) [callback]
  storage ->> storage: remember this setting is to be stored

  alt is delayed
    storage ->> timer: restart
    timer -->> storage: timeout
  end
  loop on every setting to be stored
    storage ->>+ any: call getter at service class
    any -->>- storage: current setting value
    storage ->> storage: save
  end
```

[Render this diagram at mermaid.live](https://mermaid.live/view#pako:eNplUsuS0zAQ_BXVnKBwUrEdxxsd9gIcOeUG5qDIs44KWQp6pDCp_Dsj5bHlcJNa09M9rTmDtD0CB4-_IxqJX5QYnBg7w9hRuKCkOgoTmDDTM-SDdWLAZzioEV0CicIWr6_3Os524oQ7DEGZwX-IHt3t8pH9kELrvZC_fibijTAnOxxx3KNj4aA881cmo2OwbI-5DvvOZGEd0kOPWkwJY2zeMztMHX0g39f3jLHFTDJhNuYCNLmPtvbIrGF4Qjc9TMwdPKl9SjlwlgZkAxFIRVB26E5KIpNaeH_l5LiIsHg3IKNzmJK-CZ2Ejvj_PI96TwHf3EIBNNAoVE9_e05gB-FAGXbA6djjm4g6dNCZC5WKGOxuMhJ4cBELiMdehPsqzMGvvSK5O-ZsHA7A34T2dKP__27tg4C59Nt1v_KaEYG8oftsownAm0wBfoY_wKvNcl011XbVNO2mrFblpoAJeN0st6u2bLdl3dbtS7m9FPA3i6yWbUVIW61fynq9KesCBpfmTeqXf-zs9D4)

## Load settings

```mermaid
sequenceDiagram
  participant any
  participant storage
  any ->>+ storage: LoadSettings(userSetting) [callback]
    storage ->> storage: Check if the requested setting is stored
    alt is stored
      storage ->> storage: load setting
      storage ->> any: call setter with argument save=false
    end
  deactivate storage
```

[Render this diagram at mermaid.live](https://mermaid.live/view#pako:eNp1UU1TgzAQ_SuZPemInQIFSmbspXrTU2-Kh5VsaaYQakiqtdP_bgJFR0dvycv72uwRylYQcOjo1ZIq6VZipbEpFGM71EaWcofKMFSH31BnWo0Vedi9suvF4mrEOLtvUazIGKmq7sJ2pM-XS_ZUYl2_YLl99ko2Srz-W77cULllcs3Mhpj21TpDgnWDCZNdTyUxWGBtfkP_-Nau1ujyF9ENwpkv2JNIszdpNgx1ZRvyI-OebtZYdzSISfVxgrA0co-GRjMIoCHdoBTua4-eU4AbpaECuDsKWqOtTQGFOjkqWtOuDqoEbrSlAOxOOK_zJn6Cd0K6hBHTra02wPtGAbitPLbtl4B66sOw3n7LTuAak162VhngSS8BfoR34FE6mUVJlE-TJEvDaBqmARyAx8kkn2ZhlodxFmfzMD8F8NGHTCdZ5JAsms3DeJaGcQCV9vP69NMnnWXHCg)

## About digital inputs and input events

Every hardware input is assigned a single number starting from 0 and up to 127.

The state of an input is represented by a single bit,
1 meaning a pressed button, 0 meaning a released button.
So, the combined state of all inputs is represented as a 128-bit word,
where the n-th bit represents the n-th input number.
This is called an *input bitmap*.
Least significant bit is numbered as zero.
For example, the word `00000000 ... 00000101`
means that buttons 0 and 2 are pressed,
and the others are released.

An input can also be identified by a bitmap. For example,
the button number 3 can be expressed as `...01000`.

## Brief description of most relevant subsystems

For a detailed description, see the documentation generated by Doxigen.

### AnalogInput

It works in a similar way to `DigitalInput`, but for analog inputs,
which are limited to two clutch paddles with potentiometers.

### Inputs

This is the place where inputs are set up and a number assigned to them.
All input numbers are in the range [0,127].
Analog clutch paddles are set here, too.

There is a dedicated daemon that read the state of those inputs in a loop,
every few milliseconds (the *polling loop*).
Since many inputs are read at the same time,
the combined state of all of them is reported to the `inputHub` subsystem.
Nothing is reported if there are no input events, this is,
a state change since the previous iteration.

#### DigitalInput and descendant classes

The poling loop keeps the state of all inputs in a 128 bit map
(see the `uint128_t` custom type).
Each `DigitalInput` instance must behave in this way:

- If the hardware state is unknown, it must keep the input state "as is".
- If the hardware state is known, it must set or clear each individual
  bit corresponding to the assigned input numbers
  in the input state.
- It must not set or clear any other bit in the input state.

### `inputHub` and `inputMap`

Almost all the logic behind the behavior of the sim wheel
is implemented at these subsystems.

Each assigned input number will be mapped to a user-defined HID button number,
which will be reported to the hosting computer when needed.
If there is no user-defined map, it defaults to the following rule:

```text
if (alt mode engaged) then
   HID button number = (raw input number + 64) modulo 128
else
   HID button number = raw input number
```

### User interface (`ui` subsystem)

This subsystems provides a generic way to notify events to the user,
if a user interface is available, and to display telemetry data.
It does not depend on a particular hardware,
so anything could be implemented:
a single LED, sounds, an OLED, etc.
By default, it does nothing.
To provide a particular user-interface implementation,
derive a new class from `AbstractUserInterface`,
then provide instances to `ui::add()`.

All notifications are queued, serialized and executed
in a very low priority separate thread: the *frameserver* daemon.
The caller thread does not wait for them.
There is a *frameserver* daemon for each `AbstractUserInterface` instance.

If there were two or more user interfaces (for example, display and sounds),
they should be implemented in separate classes, not to mix their code.

`AbstractUserInterface` may work in two, non-exclusive, modes:

- As a simple message queue

  For user interfaces not needing a perpetual loop
  or for one-time notifications.
  This is the default behavior.
  For example:

  ```c
  void MyImpl::onStart() {
      turnLedOn();
  }

  void MyImpl::onConnected() {
      // blink 2 times
      delay(250);
      turnLedOff();
      delay(250);
      turnLedOn();
      delay(250);
      turnLedOff();
      delay(250);
      turnLedOn();
  }
  ```

- As a frame server

  For user interfaces in need of a perpetual loop
  or for persistent notifications.
  If there are no pending notifications,
  `AbstractUserInterface::serveSingleFrame()` will be called at timed intervals.
  A non-zero frames-per-second value must be returned by `getMaxFPS()`.
  For example:

  ```c++
  void MyImpl::onStart() {
    discovering = false;
  }

  void MyImpl::onBLEdiscovering() {
      discovering = true;
  }

  void MyImpl::onConnected() {
      discovering = false;
      turnLedOn();
  }

  void MyImpl::serveSingleFrame(uint32_t elapsedMs) {
    // Called one time per second (more or less)
    // For perfect timing, use elapsedMs
    if (discovering)
      switchLed();
  }

  uint8_t MyImpl::getMaxFPS()
  {
    return 1; // one frame per second
  }
  ...

  void setup()
  {
      ...
      ui::add<MyImpl>(ledPin);
      ...
  }
  ```

> [!NOTE]
> `AbstractUserInterface::onLowBattery()` is already
> called at timed intervals as long as such a condition persists.

#### Telemetry display

The telemetry display is also implemented in the `AbstractUserInterface`.
The *frameserver* daemon will call `onTelemetryData()`
when new telemetry is detected or
when no telemetry is received for 2 seconds.

There are two possible patterns to this end:

- Buffered output:

  `AbstractUserInterface::onTelemetryData()` gets called first.
  This method draws telemetry data into a frame buffer for later display.
  The double-buffer technique may be used.

  `AbstractUserInterface::serveSingleFrame()` gets called then.
  This method copies the frame buffer to the display hardware.

- Non-buffered output

  `AbstractUserInterface::onTelemetryData()` gets called
  to do all the painting.
  `AbstractUserInterface::serveSingleFrame()` does nothing.

### BatteryMonitor

This subsystem is in charge of interfacing the underlying hardware
for "state of charge" (SOC) estimation.
A daemon calculates the SOC at timed intervals according to this algorithm:

1. Measure some battery property, depending on the underlying hardware:

   - *Voltage divider* or *battery monitor*: indirect voltage.
   - *Fuel gauge*: state of charge.

2. Determine if the battery is attached or not.
3. Compute the state of charge:

   - *Voltage divider* or *battery monitor*:
     the *BatteryCalibration* subsystem translates
     a voltage into a state of charge.
   - *Fuel gauge*: computation is already done by the chip itself.

4. If there is a change in the state of charge (1% or more),
   notify the new value to the hosting PC.
5. Notify low battery levels.
6. Shutdown on very low battery levels.

#### Fuel gauges

Fuel gauges from Maxim/Analog Devices are powered from the battery itself.
As a result, they don't respond to I2C commands if the battery is not attached.
This way, we know there is no battery.

#### Battery chargers

Typically, a battery charger is also attached to the battery.
When the battery is charging, the battery monitor
can read voltages above expected
thus reporting a state of charge higher than 100%.

Additionally, some battery chargers do not provide a constant charging voltage,
but a varying voltage wave.
For example,
[BQ2407x](../../doc/hardware/esp32reference/BQ24074_datasheet.pdf)
battery chargers provide three charging phases:
conditioning, constant current, and constant voltage.

For those reasons, the firmware takes several readings from the battery monitor
trying to figure out what the situation is.

#### Battery status guess algorithm

```mermaid
flowchart TB
  start[[Determine state of charge]]
  success_q{Success?}
  out_of_bounds_q{Is SoC out of bounds?}
  no_battery(
    battery_presence=no
    charging=unknown
    wire_presence=yes
    SOC=unknown
  )
  charging(
    battery_presence=unknown
    charging=yes
    wire_presence=yes
    SOC=unknown
  )
  discharging(
    battery_presence=yes
    charging=no
    wire_presence=unknown
    SOC=as_determined
  )

  start --> success_q
  success_q -->|no| no_battery
  success_q -->|yes| out_of_bounds_q
  out_of_bounds_q -->|yes| charging
  out_of_bounds_q -->|no| discharging
```

[Render this diagram at mermaid.live](https://mermaid.live/view#pako:eNqVUk1PwkAQ_SubOWkCBOSzTdADeDDRmMhNSpqlOy2NsIv7EUTgv7vb0kJVYjy02XnzZt7svtlBJBiCD_FSbKIFlZo8vgScEKXteTodo0a5Sjk6QCMRMXGsBGezjGWiCJUK33eT_HR3cLAwOhRxOBeGM5d8UGQiRg52DXI4Z3IRzqm2GtsrFxJyjMK1RIU8wiEXeSKTTXkyNPyNiw3P0U0q8cTdosrhyfPonHftfkWDS0KVvqVa2fIfSixVf4mV9aVQcc-qTmUop0VVyApP2FGx9IvU67cnTyoGudSei_3Zi__M26n23837xc8Tt5j-EskJnr0G1GBlR6cpsxu3c0UB6AWuMADfHhnG1Cx1AAE_WCo1Wky2PAJfS4M1MGtmV3Cc0kTSVRW8Z6kWssCkMMkC_JgulY3WlL8KURZgRn3Ktz5bfluAnKEc2ck1-N2sBPwdfIDf6zY67X7H63tet9Ue9Fs12ILvdRvtXqvZHvTs53m9m0MNPjONZmPQtw0S6a7oBA9fU6krQA)

Assumptions:

- The battery charger could put voltage on the battery pads even
  if there is no battery, so `battery_presence=unknown`.
- If the battery is present and discharging,
  `wire_presence=unknown` because the wire could be plugged in but
  not charging the battery right now.

### BatteryCalibration

Provides an estimation of the "state of charge"
given an indirect battery voltage.

#### Most accurate algorithm

Battery calibration is required for accurate battery levels.
Calibration goes from full charge to battery depletion,
taking a sample of battery voltage every minute.
All possible voltages are divided into 32 evenly distributed ranges,
called *quantum*.
Calibration data is just a set of counters of voltage samples for each quantum.
The sum of all counters is equivalent to 100% battery charge.
Calibration data is stored in flash memory.

Let's be $V_{min}(i)$ the minimum voltage that falls into quantum $i$
(a natural number),
$a < b \iff V_{min}(a)<V_{min}(b)$.
Let's be $QSIZE = V_{min}(i+1)-V_{min}(i)+1$ (the same for every quantum).
Let's be $S(i)$ the count of samples for quantum number $i$.
Let's say we have a battery voltage $V_n$ that falls
into the quantum number $n$ (0-index).

$BatteryLevel = \frac{ (\sum_{i=0}^{n-1}S(i)) +
\frac{S(n)*(V_n-V_{min}(n))}{QSIZE} }{ \sum_{j=0}^{31}S(j) } * 100$

Note that "most accurate" does not mean "accurate".
Battery voltage is not enough for accurate SOC.

#### Auto-calibrated algorithm

If calibration data is not available,
a rough estimation is provided based on LiPo batteries
characterization data taken from here:
[https://blog.ampow.com/lipo-voltage-chart/](https://blog.ampow.com/lipo-voltage-chart/).
However, actual battery voltages may not match the characterization data due to:

1) inaccurate ADC readings,
2) voltage drop due to the involved transistors (if any) and
3) unexpected impedances at the voltage divider.

For this reason, the highest voltage ever read is taken as an auto-calibration parameter.
The expected voltage reading is mapped linearly to the absolute maximum voltage ever read.
The battery needs a full charge before this algorithm provides any meaningful result.

(See [LiPoBatteryCharacterization.ods](./LiPoBatteryCharacterization.ods))

### Hid

All data interchange between the device and the host computer
is conducted through the HID protocol. This involves:

- State of buttons, axes and the alike.
- Device capabilities.
- Configuration: clutch paddles, "ALT" buttons, battery calibration, etc.
- Telemetry and pixel control.

See the [HID notes](./HID_notes.md) for more details.

### Event processing

Input events are captured in the **Input poll daemon**:
it checks the state of all inputs every 50 ms (more or less).
This period is short enough not to miss any event,
but long enough to prevent other threads from starvation.
It also plays a critical role in debouncing
(bouncing occurs during the subsequent 30 milliseconds, more or less,
after a mechanical switch is activated).

Event processing takes long, so later input events would be missed
while processing sooner ones.
To prevent this, input events are posted into a queue.

```mermaid
flowchart LR
  IP((Input poll daemon))
  Q[input event queue]
  IH((Input hub daemon))
  IP -- event --> Q
  Q -- event --> IH
```

[Render this diagram at mermaid.live](https://mermaid.live/view#pako:eNpVjssOgjAQRX-lmRUk9AdYuNKEJi5Al9ZFpYM06QObqcYQ_l1AXbCbnHvuzYzQBo1QQmfDq-1VJHY8Sc-YqLNM-CERG4K1TCt0wef5EjUXswb4RE_skTDhda1U_0qfbpuGqBnnP5_zHWvWmS0TFRTgMDpl9PzPuCgSqEeHEsr51NipZEmC9NOsqkTh_PYtlBQTFpAGrQj3Rt2jclt40IZC_LLpAwzyT0k)

Event capture is detached from event processing at the **input hub daemon**,
which runs most of the code.
Note that such a daemon is implemented inside `inputs.cpp`,
not `inputHub.cpp`.

Raw inputs are transformed into a HID input report
in a sequence of "filters" or steps:

1. Decode input from rotary coded switches, if any.
2. Detect and execute user commands, if any.
   If a command is detected and executed, this sequence is interrupted.
3. Depending on user settings,
   transform analog axis input into buttons input or vice-versa.
4. Execute clutch bite point calibration when requested.
5. Determine if ALT mode is engaged.
6. Compute F1-style clutch position.
7. Transform DPAD inputs into navigational input, depending on user settings.
8. Detect the neutral gear "virtual" button.
9. Map raw button inputs into user-defined inputs,
   if any, or use default mapping.

#### A note on rotary encoders

Each detent of a rotary encoder generates two input events in quick succession:
a button press and then, release.
Decoding is implemented by hardware interrupts,
but input events are read in the *input poll daemon*.
In summary:

1. `DT` and `CLK` signals are decoded in an interrupt service routine.
   If a rotation event is detected (clockwise or counter-clockwise),
   that event is pushed into a simple bit-oriented queue.
2. The *input poll daemon* extracts an event from the queue,
   then modifies the state of the corresponding button as pressed.
3. At the next iteration, it will reset the state of that button as non-pressed,
   thus simulating a press-then-release sequence of events.

The bit-oriented queue shows the following properties:

- Implemented as a [circular buffer](https://en.wikipedia.org/wiki/Circular_buffer).
- Thread-safe.
- Unnoticeable memory footprint.
- Size for 64 rotation events.
  If the queue were full, latest events would be discarded.
- Since each rotary is polled every 50 ms,
  it is unlikely for the queue to get full.

## About automatic shutdown

When there is no Bluetooth connection,
the systems goes to advertising.
If no BLE or USB connection is made in a certain time lapse,
the system goes to deep sleep or power off.

## About connectivity

The firmware relies in the
[HID](https://en.wikipedia.org/wiki/Human_interface_device)
standard to provide connectivity.
The device will appear as a
[Gamepad](https://en.wikipedia.org/wiki/Gamepad) to the hosting computer.
The *hid* namespace is in charge of that.

### Relevant BLE implementation details

- Regardless of the underlying stack,
  no characteristic should be notified before the client is subscribed to.
  However, the value should be set an made available for reading.
  Otherwise, notifications get randomly disabled on MS Windows machines.
  It took 4 years to hunt this bug.
  Already paired devices don't resubscribe.
  In that case, the stack stores and restores the subscription state.
  It happens *after* the device is connected.

- The *Battery Level* characteristic is mandatory even if the system
  does not have a battery.

- The Battery Service specification mandates to report a 0% battery level
  if the state of charge is unknown.
  This rule was not respected in previous versions of this project.

- The Battery Service must be initialized to 100% battery level
  (and wired status). Otherwise,
  non-battery-operated systems may cause a weird "low battery"
  warning in the hosting PC.

- In summary, we interpret that non-battery-operated systems have
  a constant, known state of charge of 100%.

- The *Battery Level Status* characteristic,
  which is optional in the Battery Service specification,
  has been implemented to report charging status, battery and wire presence.
  However, MS Windows does not take advantage of this information yet,
  nor any app known to me (except for *nRF Connect*).

- The *NimBLE* stack handles 0x2902 descriptors automatically and transparently,
  but the *Bluedroid* stack requires those descriptor to be
  explicitly created in code.

## Concurrency

System concurrency comes from these OS task and daemons:

- *Main task*: Performs initialization, then goes dormant.
- *Input poll daemon*. May call:
  - `inputs` and auxiliary classes.
  - `storage`.
- *Input hub daemon*. May call:
  - `inputHub`
  - `inputMap`
  - `inputs`
  - `hid`
  - `storage`
- *Battery monitor daemon*. May call:
  - `batteryMonitor`
  - `batteryCalibration`
  - `hid`
  - `ui`
  - `storage`
  - `power`
- *OS timers*. May call:
  - `inputs`
  - `storage`
  - `ui`
- *Bluetooth/USB stack*. May call:
  - `hid`
  - `inputs`
  - `batteryMonitor`
  - `batteryCalibration`
  - `pixels`
  - `storage`
  - `ui`
- *Frameserver*. May call:
  - `ui`
  - The `AbstractUserInterface` descendants (which may call `pixels`)

*Notes*:

- There is no synchronization between the *frameserver*
  and the *Bluetooth/USB stack*,
  except for the basic atomicity of 32-bit writes.
  Performance takes precedence over consistency.
  Only the *Bluetooth/USB stack* updates the `telemetry::data` variable.
  `telemetry::data.frameID` is always written the last.
  The *frameserver* looks for a change in that field
  in order to invoke `AbstractUserInterface::onTelemetryData()`.
