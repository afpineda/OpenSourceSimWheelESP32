/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-08
 * @brief Dump, then clear, calibration data. See [README](./README_en.md)
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "SimWheel.h"

// ----------------------------------------------------------------------------
// Auxiliary
// ----------------------------------------------------------------------------

void dumpCalibrationData()
{
    Serial.print("{ ");
    uint8_t index = 0;
    int data = batteryCalibration::getCalibration(index);
    while (data >= 0)
    {
        if (index > 0)
            Serial.print(", ");
        Serial.print(data);
        index++;
        data = batteryCalibration::getCalibration(index);
    }
    Serial.println(" };");
}

// ----------------------------------------------------------------------------
// Arduino entry point
// ----------------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;

    batteryCalibration::begin();
    Serial.println("[EN] Current battery calibration data:");
    Serial.println("[ES] Datos actuales de calibracion de bateria:");
    Serial.println("----------------------------------------------");
    dumpCalibrationData();
    Serial.println("----------------------------------------------");
    delay(50);
    Serial.println("[EN] This data will be ERASED in 60 seconds. Power off to abort.");
    Serial.println("[ES] Estos datos seran BORRADOS en 60 segundos. Apague para abortar.");
    for (int countdown=60; countdown>0; countdown--) {
        Serial.print(countdown);
        Serial.print("...");
        delay(1000);
    }
    Serial.println("0");
    batteryCalibration::clear();
    batteryCalibration::save();
    Serial.println("[EN] Battery calibration data has been ERASED");
    Serial.println("[ES] Los datos de calibracion de bateria han sido ELIMINADOS");
    Serial.println("--END--FIN--");
}

void loop()
{
    delay(5000);
}