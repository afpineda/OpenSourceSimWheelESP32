# Unit test: power on/off through a power latch circuit

## Purpose and summary

To test power off.

## Warning

In this particular case, **under no circumstances should you power the DevKit board with the powerboost module/shield nor the batteries**. The board could get damaged. The system will get power from the USB cable.

## Harware setup

The [power latch circuit](../../../../doc/hardware/subsystems/PowerLatch/PowerLatchSubsys_en.md) must be in place along with the [powerboost module/shield](../../../../doc/hardware/subsystems/PowerBoost/Powerboost_en.md). However, we are not taking power from the 

Actual GPIO pins are defined at [debugUtils.h](debugUtils.h). Wire "POWER_LATCH" (from the latch circuit) to TEST_POWER_PIN (at the DevKit board).

Output through USB serial port at 115200 bauds.

## Procedure and expected output

### Uploading

Note that the test sketch can not be uploaded unless the board is powered up. This will not happen while the power latch circuit is in place (unless you hold the power button all the time) because there is no software to enable the "keep power on" pin. In order to upload the sketch, disconnect the DevKit board from the rest of the circuit. 

1. Just extract it from the protoboard.
2. Plug the USB cable.
3. Compile and upload the test sketch.
4. Unplug the USB cable.
5. Place the DevKit board in the protoboard again.
6. Plug the USB cable again (note that the board should not get power this time).

### Testing

1. Since there is no power, no output will be shown, and the proper COM port will be hidden to the PC side.
2. Press and hold the power button at the latch circuit. 
3. Ensure the proper COM port appears at the PC side and connect the serial terminal to this port. You have 20 seconds to do so, otherwise restart this procedure. Some of the following output may be missed, but the countdown should be noticeable:
   
   ```
   --READY--
   Going to power off in 20 seconds
   ...20...19...18...17...16...15...14...13...12...11...10...9...8...7...6...5...4...3...2...1...0
   Power off
   ```
4. Wait for the countdown to complete.
5. Wait for 5 seconds or more. **No other output** must appear. The proper COM port should disappear at the PC side.
6. Reset. Nothing should happen.
