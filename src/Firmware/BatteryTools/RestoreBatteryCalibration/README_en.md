# Restore battery calibration data

A copy of the calibration data is needed. Calibration data is a set of 32 numbers enclosed in curly brackets and separated by commas.

## Purpose

To restore battery calibration data in a variety of circumstances:

- Calibration data obtained from another user with the same battery model.
- Moving from one DevKit board to another.
- Moving from one battery model to another.
- Flash memory overwritten due to another sketch.
- Failed calibration procedure.
- Other.

## Procedure

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
const uint16_t customCalibrationData[] = { 0, 0, 0, 0, 0, 0, 0, 0, 122, 254 , 250, 112, 52, 25, , 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; 
```

4. Save the file, compile and upload the sketch to the DevKit board. 
5. Open the serial monitor. 
6. Reset. 
7. Output should be:
   
   ```
   [EN] Battery calibration data has been restored
   [ES] Los datos de calibracion de bateria han sido restaurados
   --END--FIN--
   ```

There is no need to run this sketch more than once.
