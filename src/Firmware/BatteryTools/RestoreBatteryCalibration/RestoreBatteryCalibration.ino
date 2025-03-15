/**
 * @file RestoreBatteryCalibration.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-08
 * @brief Restore battery calibration data. See [README](./README_en.md)
 *
 * @copyright Licensed under the EUPL
 *
 */
#include <HardwareSerial.h>

#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"

// ----------------------------------------------------------------------------
// CALIBRATION DATA
// ----------------------------------------------------------------------------

// [EN] Uncomment and paste your backup data after "="
// [ES] Descomente y pegue sus datos de respaldo despues de "="

// const uint16_t customCalibrationData[] =

// ----------------------------------------------------------------------------
// Auxiliary
// ----------------------------------------------------------------------------

void restoreCalibrationData()
{
    internals::batteryCalibration::clear();
    for (uint8_t i = 0; i < sizeof(customCalibrationData); i++)
        BatteryCalibrationService::call::setCalibrationData(
            i,
            customCalibrationData[i],
            false);
    SaveSetting::notify(UserSetting::BATTERY_CALIBRATION_DATA);
}

// ----------------------------------------------------------------------------
// Arduino entry point
// ----------------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    internals::storage::getReady();
    internals::batteryCalibration::getReady();
    OnStart::notify();

    // Check data array
    int len = sizeof(customCalibrationData) / sizeof(uint16_t);
    if (len != BatteryCalibrationService::call::getCalibrationDataCount())
    {
        Serial.println("[EN] ERROR: calibration data does not contain the expected 32 numbers");
        Serial.println("[ES] ERROR: los datos de calibracion no contienen los 32 numeros esperados");
    }
    else
    {
        restoreCalibrationData();
        Serial.println("[EN] Battery calibration data has been restored");
        Serial.println("[ES] Los datos de calibracion de bateria han sido restaurados");
    }
    Serial.println("--END--FIN--");
}

void loop()
{
    delay(5000);
}