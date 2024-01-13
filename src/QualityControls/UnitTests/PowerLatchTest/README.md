# Unit test: power on/off through a power latch circuit

## Purpose and summary

To test power off using an external latch circuit.

## Warning

In this particular case, **under no circumstances should you power the DevKit board with the powerboost module/shield nor the batteries**. The board could get damaged. The system will get power from the USB cable.

## Hardware setup

Actual GPIO numbers are defined at [debugUtils.h](./debugUtils.h).

A [powerboost module/shield](../../../../doc/hardware/subsystems/PowerBoost/Powerboost_en.md) must be in place equipped with a latch circuit. However, we are not taking power from it. The constant `TEST_LATCH_MODE`, at [debugUtils.h](./debugUtils.h), may need modification depending on how it works. Wire `POWER_LATCH` (from the latch circuit) to `TEST_POWER_PIN` (at the DevKit board).

If the powerboost module does not have a power-on LED, use a multimeter to know if it is powered on or off.

Output through USB serial port at 115200 bauds.

## Procedure and expected output

1. Ensure the powerboost module is on.
2. Reset. Ignore output from the operating system itself.
3. A countdown will show up:

   ```text
   --READY--
   Going to power off in 20 seconds
   ...20...19...18...17...16...15...14...13...12...11...10...9...8...7...6...5...4...3...2...1...0
   Power off
   ```

4. Check that the powerboost module is still on before the countdown expires.
5. Wait for the countdown to complete.
6. Check that the powerboost module is off.
