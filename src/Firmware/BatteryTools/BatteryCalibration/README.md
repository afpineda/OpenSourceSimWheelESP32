# Battery calibration procedures

## Hardware setup

Those procedures are suitable for any circuit board as long as the [Battery monitor subsystem](../../../../doc/hardware/subsystems/BatteryMonitor/BatteryMonitor_en.md) is implemented. Obviously, the [power](../../../../doc/hardware/subsystems/Power/Power_en.md) subsystem must be based on batteries.  Actual GPIO numbers are defined inside the [sketch](./BatteryCalibration.ino). You must edit them to fit your setup. They are: `BATT_EN_PIN` and `BATT_READ_PIN`. For example:

```c
#define BATT_EN_PIN GPIO_NUM_17
#define BATT_READ_PIN GPIO_NUM_15
```

If a simple voltage divider is used as a battery monitor, the first "define" must show:

```c
#define BATT_EN_PIN -1
```

**Warning**: If your DevKit does not feature a built-in powerboost, **never plug the USB cable and the external powerboost at the same time**.

## Battery calibration

**Note**: completing this procedure may take hours or days, however, no human supervision is required. This procedure is **optional**, but highly recommended.
Please, read from start to end before proceeding.

### Purpose

Due to non-linearity of battery charge versus battery voltage, only a rough and imprecise estimation can be achieved. This procedure collects calibration data in order to provide a better estimation of battery charge for your particular battery. Data is saved to flash memory, so it can be reused later at any other sketch. This procedure will go through a complete discharge cycle.

Use [Online calculator: Battery discharge time depending upon load](https://planetcalc.com/2283/) to estimate how long will it take to deplete the battery.

*Warning:* if `BATT_READ_PIN` is not properly attached to the positive pole of the battery, no data will be collected, thus this procedure will be useless.

### Calibration procedure

1. **Ensure the battery is fully charged before continuing**.
2. Plug the USB cable.
3. Upload the sketch ([BatteryCalibration.ino](./BatteryCalibration.ino)) to the DevKit board.
4. Unplug the USB cable.
5. If your board has built-in battery support, push the reset button. If not, the board has no power, so attach the *powerboost* module properly.
6. Make sure the on-board LED is on, so the DevKit board has power.
7. *Note:* the sketch will wait for 1 minute before continuing, to avoid accidental overwrite of calibration data.
8. Wait for the battery to deplete. The on-board LED will go off.

The calibration data is now saved to flash memory and ready to use. However, you should follow the *Backup procedure* below.

If the calibration procedure is interrupted, you should start again, otherwise wrong battery levels will be reported.

Since there is no serial connection, in order to check if the sketch is running properly:

- Wait for one minutes after power on.
- Open the bluetooth control panel at the host PC.
- Ensure the device is **not** paired due to a previous sketch.
- Click on "Add device".
- The board should get listed as "Battery calibration".

### Further action

You **should not run the sketch again**, otherwise the acquired data will get cleared.
You have **one minute to cut power** before this happens.
Take this into account if you plug another charged battery. However, there is no risk if you plug the USB cable.
Upload another sketch as soon as possible.

## Backup of calibration data

### Purpose

As a backup measure, calibration data is dumped to the serial port, so you can copy-paste it into the [restoration sketch](../../BatteryTools/RestoreBatteryCalibration/README.md) in case of need. This way, the calibration procedure does not need to be run more than once. You may also contribute this data to the community, but take into account that it will not work with a different battery monitor circuit or another battery model.

### Backup procedure

1. **Plug the USB cable**.
2. If not done yet, upload the sketch ([BatteryCalibration.ino](./BatteryCalibration.ino)) to the DevKit board.
3. Open the serial monitor (115200 bauds).
4. Reset.
5. Output will show:

    ```text
    [EN] Current battery calibration data:
    [ES] Datos actuales de calibracion de bateria:
    ----------------------------------------------
    ```

6. The next line is the calibration data. Copy-paste to a text file and save it. For example:

   ```text
   { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 4, 11, 64, 132, 54, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
   ```

7. Ignore further output.

### Extra ball

The sum of all those numbers will give you the capacity of the fully charged battery, in minutes, more or less.
