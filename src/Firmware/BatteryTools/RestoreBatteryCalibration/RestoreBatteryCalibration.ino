/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-08
 * @brief Restore battery calibration data. See [README](./README_en.md)
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */
#include <Arduino.h>
#include "SimWheel.h"


// ----------------------------------------------------------------------------
// CALIBRATION DATA
// ----------------------------------------------------------------------------

// [EN] Uncomment and paste your backup data after "="
// [ES] Descomente y pegue sus datos de respaldo despues de "="

// const uint16_t customCalibrationData[] = ;

// ----------------------------------------------------------------------------
// Auxiliary
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Arduino entry point
// ----------------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;
    
    // Check data array
    int len = sizeof(customCalibrationData)/sizeof(uint16_t);
    if (len!=32) {
        Serial.println("[EN] ERROR: calibration data does not contain the expected 32 numbers");
        Serial.println("[ES] ERROR: los datos de calibracion no contienen los 32 numeros esperados");
    } else {
        batteryCalibration::restoreCalibrationData(customCalibrationData);
        batteryCalibration::save();
        Serial.println("[EN] Battery calibration data has been restored");
        Serial.println("[ES] Los datos de calibracion de bateria han sido restaurados");
    }
    Serial.println("--END--FIN--");
}

void loop()
{
    delay(5000);
}