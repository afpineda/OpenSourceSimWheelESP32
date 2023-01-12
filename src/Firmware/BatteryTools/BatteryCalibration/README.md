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

Due to non-linearity of battery charge versus battery voltage, only a rough and imprecise estimation can be achieved. This procedure collects calibration data in order to provide a better estimation of battery charge for your particular battery. Data is saved to flash memory, so it can be reused later at any other sketch. This procedure will go through a complete discharge cycle.

Use [Online calculator: Battery discharge time depending upon load](https://planetcalc.com/2283/) to estimate how long will it take to deplete the battery.

*Warning:* if `BATT_READ_PIN` is not properly attached to the positive pole of the battery, calibration data will be cleared but not collected, thus this procedure will be useless.

### Calibration procedure

- If your DevKit **does NOT feature a built-in powerboost module**, so you are using an external one:

  1. **Ensure the battery is fully charged before continuing**.
  2. Unplug the battery (or the external powerboost module).
  3. Plug the USB cable.
  4. Upload the sketch ([BatteryCalibration.ino](./BatteryCalibration.ino)) to the DevKit board (if not done yet).
  5. Unplug the USB cable (the DevKit has no power right now).
  6. **Plug the battery** or the external powerboost module. Ensure `BATT_READ_PIN` is properly attached, too.
  7. Make sure the on-board LED is on, so the DevKit board has power.
  8. Wait for the battery to deplete. The on-board LED will go off.

- If your DevKit **has built-in battery support** with a built-in powerboost module:

  1. **Ensure the battery is fully charged before continuing**.
  2. Plug the USB cable and the battery.
  3. Upload the sketch ([BatteryCalibration.ino](./BatteryCalibration.ino)) to the DevKit board (if not done yet).
  4. **Unplug the USB cable** (the DevKit is powered through the battery right now).
  5. **Reset**.
  6. Make sure the on-board LED is on, so the DevKit board has power.
  7. Wait for the battery to deplete. The on-board LED will go off.

The calibration data is now saved to flash memory and ready to use. However, you should follow the *Backup procedure* below.

If the calibration procedure is interrupted, you should start again with a fully charged battery, otherwise wrong battery levels will be reported.

Since there is no serial connection, in order to check if the sketch is running properly:

- Wait for three minutes after power on (or reset).
- Open the bluetooth control panel at the host PC.
- Ensure the device is **not** paired due to a previous sketch.
- Click on "Add device".
- The device should get listed as "Battery calibration". Do not pair.

### Further action

You **should not run the sketch again**, otherwise the acquired data will be cleared.
You have **three minutes to cut power** before this happens.
Take this into account if you plug another charged battery or the USB cable.
Upload another sketch as soon as possible. Follow the backup procedure below to avoid any risk before uploading.

## Backup of calibration data

As a backup measure, calibration data is dumped to the serial port, so you can copy-paste it into the [restoration sketch](../../BatteryTools/RestoreBatteryCalibration/README.md) in case of need. This way, the calibration procedure does not need to be run more than once. You may also contribute this data to the community, but take into account that it will not work with a different battery monitor circuit or another battery model.

You should also follow this procedure to avoid accidental overwrite of calibration data while the sketch is still in flash memory.

### Backup procedure

1. If your DevKit does not feature a built-in powerboost module, unplug the battery.
2. **Plug the USB cable**.
3. If not done yet, upload the sketch ([BatteryCalibration.ino](./BatteryCalibration.ino)) to the DevKit board.
4. Open the serial monitor (115200 bauds):

   - If you are using *Arduino IDE*, press "CTRL+SHIFT+M".
   - If you are using *Visual Studio Code*, click on the small plug-shaped icon near the lower-right corner of the window.

5. Reset. Output will show:

   ```text
   Waiting...
   ```

6. At the serial monitor, send any character before 3 minutes elapses:

   - If you are using *Arduino IDE*, press the "SEND" button.
   - If you are using *Visual Studio Code*, press "F1" and type "Arduino: send text to serial port". Select that command. Type any character, then press "ENTER".

7. Output will show:

   ```text
   [EN] Current battery calibration data:
   [ES] Datos actuales de calibracion de bateria:
   ----------------------------------------------
   ```

8. The next line is the calibration data. Copy-paste to a text file and save it. For example:

   ```text
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 5, 35, 122, 103, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   ```

9. Ignore further output. At this point, the sketch will not collect new calibration data.

### Extra ball

The sum of all those numbers will give you the capacity of the fully charged battery, in minutes, more or less.
