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

#include <Arduino.h>
#include "SimWheel.h"

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

extern int getBatteryReadingForTesting(gpio_num_t battENPin, gpio_num_t battREADPin);

// ----------------------------------------------------------------------------
// Auxiliary
// ----------------------------------------------------------------------------

extern void configureBatteryMonitor(
    gpio_num_t enableBatteryReadPin,
    gpio_num_t batteryLevelPin);

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
// Mocks
// ----------------------------------------------------------------------------

void inputs::update()
{
}

void inputs::recalibrateAxes()
{
}

void inputs::reverseLeftAxis()
{
}

void inputs::reverseRightAxis()
{
}

void inputs::setRotaryPulseX1() {}

void inputs::setRotaryPulseX2() {}

void inputs::setRotaryPulseX3() {}

// ----------------------------------------------------------------------------
// Arduino entry point
// ----------------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);

    int countdown = 3 * 60;
    // int countdown = 10;
    Serial.begin(115200);

    configureBatteryMonitor(BATT_EN_PIN, BATT_READ_PIN);
    if (getBatteryReadingForTesting(BATT_EN_PIN, BATT_READ_PIN) < 150)
    {
        Serial.println("Note: No battery detected.");
    }

    Serial.println("Waiting...");
    while ((!Serial.available()) && (countdown > 0))
    {
        countdown--;
        delay(1000);
    }

    if (countdown <= 0)
    {
        Serial.println("Running...");
        Serial.println("If you can see this message, unplug the USB cable,");
        Serial.println("plug a fully charged battery and reset.");
        batteryCalibration::clear();
        batteryCalibration::save();
        Serial.println("Calibration data has been cleared.");
        hidImplementation::begin("Battery calibration", "Mamandurrio", false);
    }
    else
    {
        batteryCalibration::begin();
        Serial.println("[EN] Current battery calibration data:");
        Serial.println("[ES] Datos actuales de calibracion de bateria:");
        Serial.println("----------------------------------------------");
        dumpCalibrationData();
        Serial.println("----------------------------------------------");
        Serial.println("--END--FIN--");
        Serial.end();
        for (;;)
            delay(1000);
    }
}

void loop()
{
    int reading = getBatteryReadingForTesting(BATT_EN_PIN, BATT_READ_PIN);
    if (reading >= 150)
        batteryCalibration::addSample(reading, true);
    // else BATT_READ_PIN is not connected
    delay(SAMPLING_MILLIS);
}