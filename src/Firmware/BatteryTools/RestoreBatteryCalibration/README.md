# Restore battery calibration data

A copy of the calibration data is needed. Calibration data is a set of 32 numbers enclosed in curly brackets and separated by commas.
For example:

```text
{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 4, 11, 64, 132, 54, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
```

## Purpose

To restore battery calibration data in a variety of circumstances:

- Calibration data obtained from another user with the same battery model and battery monitor subsystem.
- Moving from one DevKit board to another (same model).
- Moving from one battery model to another (already calibrated before).
- Flash memory overwritten due to another sketch.
- Failed calibration procedure.
- Other.

## Restoration procedure

1. Edit [RestoreBatteryCalibration.ino](RestoreBatteryCalibration.ino) and locate the following line:

   ```c
   //const uint16_t customCalibrationData[] = 
   ```

2. Uncomment that line:

   ```c
   const uint16_t customCalibrationData[] = 
   ```

3. And write the calibration data, bracket to bracket, after `=`. End with `;`. For example:

   ```c
   const uint16_t customCalibrationData[] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 4, 11, 64, 132, 54, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }; 
   ```

4. Save the file, compile and upload the sketch to the DevKit board.
5. Open the serial monitor.
6. Reset.
7. Output should be:

   ```text
   [EN] Battery calibration data has been restored
   [ES] Los datos de calibracion de bateria han sido restaurados
   --END--FIN--
   ```

There is no need to run this sketch more than once.

## Cleaning procedure

For testing purposes, calibration data may be cleared just by using this calibration data:

```c
const uint16_t customCalibrationData[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; 
```
