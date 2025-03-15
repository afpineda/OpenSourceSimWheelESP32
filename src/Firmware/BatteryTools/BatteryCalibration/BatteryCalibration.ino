/**
 * @file BatteryCalibration.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-08
 * @brief Data acquisition for battery calibration. See [README](./README_en.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include "HAL.hpp"

#include <HardwareSerial.h>

// ----------------------------------------------------------------------------
// GPIO
// ----------------------------------------------------------------------------

// [EN] Put your GPIO pin numbers here
// [ES] Ponga aquí sus números de pin GPIO

#define BATT_EN_PIN GPIO_NUM_42
#define BATT_READ_PIN GPIO_NUM_2

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

#define SAMPLING_MILLIS (60 * 1000) // 1 minute

// ----------------------------------------------------------------------------
// Auxiliary
// ----------------------------------------------------------------------------

void dumpCalibrationData()
{
    int data;
    Serial.print("{ ");
    for (
        uint8_t index = 0;
        index < BatteryCalibrationService::call::getCalibrationDataCount();
        index++)
    {
        data = BatteryCalibrationService::call::getCalibrationData(index);
        if (index > 0)
            Serial.print(", ");
        Serial.print(data);
    }
    Serial.println(" };");
}

int getBatteryReading(ADC_GPIO batteryREADPin, OutputGPIO battENPin)
{
    if (battENPin != UNSPECIFIED::VALUE)
    {
        GPIO_SET_LEVEL(battENPin, 1);
        DELAY_TICKS(200);
    }
    int reading = internals::hal::gpio::getADCreading(batteryREADPin, 100);
    if (battENPin != UNSPECIFIED::VALUE)
    {
        GPIO_SET_LEVEL(battENPin, 0);
    }
    return reading;
}

// ----------------------------------------------------------------------------
// Mocks
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// Arduino entry point
// ----------------------------------------------------------------------------

ADC_GPIO battRead;
OutputGPIO battEN;

void setup()
{
    int countdown = 3 * 60;
    // int countdown = 10;
    Serial.begin(115200);

    battRead = BATT_READ_PIN;
    battEN = BATT_EN_PIN;
    batteryMonitor::configure(battRead, battEN);
    hid::configure("Battery calibration", "Mamandurrio", false);
    internals::hid::common::getReady();
    internals::storage::getReady();
    internals::batteryCalibration::getReady();
    OnStart::notify();

    if (getBatteryReading(battRead, battEN) < 150)
    {
        Serial.println("Note: No battery detected.");
    }

    Serial.println("Waiting...");
    while ((!Serial.available()) && (countdown > 0))
    {
        countdown--;
        DELAY_MS(1000);
    }

    if (countdown <= 0)
    {
        Serial.println("Running...");
        Serial.println("If you can see this message, unplug the USB cable,");
        Serial.println("plug a fully charged battery and reset.");
        internals::batteryCalibration::clear();
        SaveSetting::notify(UserSetting::BATTERY_CALIBRATION_DATA);
        Serial.println("Calibration data has been cleared.");
    }
    else
    {
        Serial.println("[EN] Current battery calibration data:");
        Serial.println("[ES] Datos actuales de calibracion de bateria:");
        Serial.println("----------------------------------------------");
        dumpCalibrationData();
        Serial.println("----------------------------------------------");
        Serial.println("--END--FIN--");
        Serial.end();
        for (;;)
            DELAY_MS(1000);
    }
}

void loop()
{
    int reading = getBatteryReading(battRead, battEN);
    if (reading >= 150)
    {
        internals::batteryCalibration::addSample(reading);
        SaveSetting::notify(UserSetting::BATTERY_CALIBRATION_DATA);
    } // else BATT_READ_PIN is not connected
    DELAY_MS(SAMPLING_MILLIS);
}