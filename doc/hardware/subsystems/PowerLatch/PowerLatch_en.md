# Power latch subsystem

**You don't build this subsystem**, it may come equipped with your powerboost module/shield.

## Purpose

The purpose of this subsystem is to completely **cut power off** to the system when required, thus saving battery charge. This subsystem is **optional**. When not in place, the system will enter [**deep sleep**](https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/) mode instead, which drains current, but at a lower rate.

- There is no need for a power latch circuit if the system is powered through the wheel's base or any other external power source not based on batteries.
- This subsystem requires a single dedicated output pin (GPIO) at the DevKit board, named `POWER_LATCH`. This pin does not require to be attached to anything, so it can be reserved for future use. 

## External power latch circuit

Your powerboost module/shield may come equipped with this subsystem. You know that because there is a push button (**but not a switch**) to turn on/off the module. One of the two terminals where the push button is soldered will work as `POWER_LATCH`, so a wire has to be soldered there. To know which one, just do some testing using a wire. Connect a terminal to `GND` and wait a few seconds. If power goes on/off, that terminal is `POWER_LATCH`.

## Firmware customization

Customization takes place at file [CustomSetup.ino](../../../../src/Firmware/CustomSetup/CustomSetup.ino).

### No power latch circuit

A "wake up source" is mandatory if there is no power latch circuit, this is, a set of GPIO numbers. It is recommended even if there is a power latch circuit, as a fallback measure in case the power latch is not wired. Anything wired to the button matrix is unable to act as a wake up source, since the button matrix will not be powered while in deep sleep. There are some choices:

#### Wake up source: RESET button

With this choice, a reset push button is the only way to wake up from deep sleep: one terminal wired to `GND` and the other one wired to the `EN` pin at the DevKit Board. This is **not recommended**, as the reset button may be pushed by accident, thus ruining your race.  No firmware customization is required in this case.

#### Wake up source: Rotary encoders or a dedicated push button

With this choice one or more rotary encoders may be configured. All of them have to be activated at the same time to wake up from deep sleep. This is more reliable for their built in push buttons (`SW` pin) than for rotation (`CLK` or `DT`), as long as they are not attached to the button matrix.

Locate the following line at *CustomSetup.ino*:

```c
const gpio_num_t WAKEUP_PINS[] = {};
```

Put inside the curly brackets a comma-separated list of GPIO numbers. Those GPIO numbers must be attached to a `CLK`, `DT` or `SW` pin. **Any other pulled up input may be used, too**. For example:

```c
const gpio_num_t WAKEUP_PINS[] = {GPIO_NUM_39, GPIO_NUM_32};
```

Locate the line starting with `#define WAKEUP_ANYorALL`. It must match the following:

```c
#define WAKEUP_ANYorALL false
```

If your custom setup does not use rotary encoders, you must provide a dedicated pulled up push button as a wake up source. The firmware will enable internal pull-up resistors if available. `GPIO_NUM_3` is perfect for this, since it is always pulled up.

### External power latch circuit

Locate the line that contains `#define POWER_LATCH`, and ensure it is not commented out:

```c
#define POWER_LATCH
```

Then write the assigned GPIO number to `POWER_LATCH` at the right side. For example:

```c
#define POWER_LATCH GPIO_NUM_1
```