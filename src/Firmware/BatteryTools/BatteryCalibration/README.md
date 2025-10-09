# Battery calibration procedures

## Hardware setup

Those procedures are suitable for any circuit board as long as the
[Battery monitor subsystem](../../../../doc/hardware/subsystems/BatteryMonitor/BatteryMonitor_en.md)
is implemented.
Obviously, the
[power](../../../../doc/hardware/subsystems/Power/Power_en.md)
subsystem must be based on batteries.
Actual GPIO numbers are defined inside the [sketch](./BatteryCalibration.ino).
You must edit them to fit your setup.
They are: `BATT_EN_PIN` and `BATT_READ_PIN`.
For example:

```c
#define BATT_EN_PIN GPIO_NUM_17
#define BATT_READ_PIN GPIO_NUM_15
```

If a simple voltage divider is used as a battery monitor, the first "define" must show:

```c
#define BATT_EN_PIN -1
```

Some DevKit boards feature a built-in voltage divider internally wired to a certain GPIO.
Look for a data sheet to know which one and set `BATT_READ_PIN` properly.

> [!CAUTION]
> If your DevKit does not feature built-in battery support,
> **never plug the USB cable and the external powerboost at the same time**.

## Battery calibration

> [!NOTE]
> Completing this procedure may take hours or days, however,
> no human supervision is required. This procedure is **optional**, but highly recommended.
> Please, read from start to end before proceeding.

Due to non-linearity of battery charge versus battery voltage,
only a rough and imprecise estimation can be achieved.
This procedure collects calibration data in order to provide
a better estimation of battery charge for your particular battery.
Data is saved to flash memory, so it can be reused later at any other sketch.
This procedure will go through a complete discharge cycle.

Use [Online calculator: Battery discharge time depending upon load](https://planetcalc.com/2283/)
to estimate how long will it take to deplete the battery.

### Calibration procedure (and backup)

> [!NOTE]
> No real battery level will be reported to the host computer
> during this procedure. Yoy will get a constant 66% battery level.

Please, pay attention to the following instructions.

1. Sketch uploading:

   1. Ensure your DevKit board is **not paired** to a hosting PC due to a previous firmware.
      **Remove your device from the Bluetooth control panel**.
   2. If your DevKit does not feature built-in battery support,
      **remove the battery** from the external powerboost module.
   3. Plug the USB cable into the DevKit board.
   4. Open [BatteryCalibration.ino](./BatteryCalibration.ino) using *Arduino IDE*
      and check that the `BATT_READ_PIN` and `BATT_EN_PIN` values
      are properly set as explained before.
   5. Upload [BatteryCalibration.ino](./BatteryCalibration.ino).

2. Battery calibration:

   1. **Ensure the battery is fully charged before continuing**.
      If this calibration procedure is interrupted,
      you must start again with a fully charged battery,
      otherwise wrong battery levels will be reported.
   2. Remove the USB cable. **No wired power** must be available.
   3. Ensure `battery(+)` is wired to the battery monitor circuit.
   4. Ensure the DevKit board is powered by the battery (check the onboard LED).
   5. Go to the Bluetooth control panel and pair the device.
      It should be listed as **"Battery calibration"**.
      **IF YOUR DEVICE DOES NOT SHOW UP** (in a reasonable time lapse),
      follow to the [data removal procedure](#data-removal-procedure) below,
      then repeat this battery calibration procedure.
      This is a precaution to prevent previous calibration data
      from being accidentally deleted.

   6. Battery calibration will start as soon as your device is connected to the computer.
      Afterwards, you can turn your computer off if you want.
   7. **Wait for the battery to deplete**. The onboard LED will go off.
      The calibration data is now saved to flash memory and ready to use.

3. Data backup (optional but recommended):

   1. If your DevKit does not feature built-in battery support,
      **remove the battery** from the external powerboost module.
   2. Plug the USB cable into the DevKit board.
   3. Open the serial monitor in Arduino IDE.
   4. Hit the `RESET` button on the DevKit board.
   5. Look for the text line next to
      `Current calibration data follows. Write down for backup:`.
      Copy-paste to a text file and save it.
      It looks like this:

      ```text
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 5, 35, 122, 103, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
      ```

4. Upload your custom firmware or "ready-to-deploy" sketch
   as explained in the project documentation.

## Data removal procedure

You have to *explicitly* remove the previous battery calibration data from flash memory
if you want to repeat the battery calibration procedure.
[BatteryCalibration.ino](./BatteryCalibration.ino) must be loaded into the device.

1. If your DevKit does not feature built-in battery support,
   **remove the battery** from the external powerboost module.
2. Plug the USB cable into the DevKit board.
3. Open the serial monitor in *Arduino IDE*.
4. Hit the `RESET` button on the DevKit board.
5. Follow on-screen instructions.

## Data restoration procedure

There is a restoration sketch in case of need.
This way, the calibration procedure does not need to be run more than once.
You need the list of numbers (between braces) that were printed to the serial monitor
in the third step (data backup).
[Just follow the instructions](../../BatteryTools/RestoreBatteryCalibration/README.md).

### Extra ball

The sum of all those numbers will give you the capacity of the fully charged battery, in minutes, more or less.
