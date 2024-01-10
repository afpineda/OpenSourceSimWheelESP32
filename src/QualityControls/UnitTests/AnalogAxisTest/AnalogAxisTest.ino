/**
 * @file AnalogAxisTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-03
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "SimWheel.h"
#include "debugUtils.h"
#include "PolledInput.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

AnalogAxisInput *input;
clutchValue_t oldValue = CLUTCH_INVALID_VALUE;

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    Serial.println("-- READY --");

    input = new AnalogAxisInput(TEST_ANALOG_PIN1, TEST_ANALOG_PIN1);
    input->resetCalibrationData();
    Serial.println("-- GO --");
}

void loop()
{
    clutchValue_t value;
    bool autocalibrated;

    input->read(value, autocalibrated);
    if ((oldValue != value) || autocalibrated)
    {
        serialPrintf("Axis: %d ", value);
        if (autocalibrated)
            Serial.println("(autocalibrated)");
        else
            Serial.println("");
    }
    oldValue = value;
    delay(30);
}