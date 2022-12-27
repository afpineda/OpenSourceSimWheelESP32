/**
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

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("-- READY --");

    input = new AnalogAxisInput(TEST_ANALOG_PIN1, TEST_ANALOG_PIN1);
    input->resetCalibrationData();
    Serial.println("-- GO --");
}

void loop()
{
    clutchValue_t value;
    bool autocal;
    bool changed;

    input->read(&value, &changed, &autocal);
    if (changed || autocal)
    {
        serialPrintf("Axis: %d ", value);
        if (autocal)
            Serial.println("(autocalibrated)");
        else
            Serial.println("");
    }

    delay(30);
}