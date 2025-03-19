# Unit test: power on/off through a power latch circuit

## Purpose and summary

To test power off using an external latch circuit.

> [!CAUTION]
> In this particular case,
> **under no circumstances should you power the DevKit board with the powerboost module/shield nor the batteries**.
> The board could get damaged. The system will get power from the USB cable.

## Hardware setup

There are two valid hardware setups to choose from:

1. **Using a powerboost module**:
   A powerboost module/shield must be in place equipped with a latch circuit.
   However, we are not taking power from it.
   The constant `TEST_LATCH_MODE`, at [Testing.hpp](./../../../../include/Testing.hpp),
   may need modification depending on how it works.
   This test expects the latch circuit to work in *open drain* mode.
   Wire `POWER_LATCH` (from the latch circuit) to `TEST_POWER_PIN`
   (at the DevKit board).

2. **Using a simple multimeter/polimeter**:
  If you don't have a powerboost module,
  or the powerboost module does not have a power-on LED,
  use a multimeter configured in **continuity mode**.
  Attach one probe to `GND` and the other to the
  `TEST_POWER_PIN`.

Output through USB serial port at 115200 bauds.

## Procedure and expected output

1. Ensure that either:
   - The powerboost module is on, or
   - The multimeter is properly wired.
2. Reset. Ignore output from the operating system itself.
3. A countdown will show up:

   ```text
   --READY--
   Going to power off in 20 seconds
   ...20...19...18...17...16...15...14...13...12...11...10...9...8...7...6...5...4...3...2...1...0
   Power off
   ```

4. Before the countdown expires, check that either:
   - The powerboost module is on
   - The multimeter does not detect continuity.
5. Wait for the countdown to complete.
6. When the countdown expires, check that either:
   - The powerboost module is off.
   - The multimeter detects continuity (it beeps)
     for **two seconds**, more or less.
