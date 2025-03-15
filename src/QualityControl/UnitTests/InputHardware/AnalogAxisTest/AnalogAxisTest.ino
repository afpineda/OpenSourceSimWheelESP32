/**
 * @file AnalogAxisTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-03
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "Testing.hpp"
#include "InputHardware.hpp"

#include "HardwareSerial.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

AnalogClutchInput *input;
uint8_t oldValue = CLUTCH_INVALID_VALUE;

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    Serial.println("-- READY --");

    input = new AnalogClutchInput(TEST_ANALOG_PIN1);
    input->resetCalibrationData();
    Serial.println("-- GO --");
}

void loop()
{
    uint8_t value;
    bool autocalibrated;

    input->read(value, autocalibrated);
    if ((oldValue != value) || autocalibrated)
    {
        Serial.printf("Axis: %d ", value);
        if (autocalibrated)
            Serial.println("(autocalibrated)");
        else
            Serial.println("");
    }
    oldValue = value;
    delay(30);
}