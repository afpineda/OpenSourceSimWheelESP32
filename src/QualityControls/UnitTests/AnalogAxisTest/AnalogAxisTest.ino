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

AnalogAxisInput *input1, *input2;

#define IN_1 17
#define IN_2 21

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("-- READY --");

    input1 = new AnalogAxisInput(TEST_ANALOG_PIN1, IN_1);
    input2 = new AnalogAxisInput(TEST_ANALOG_PIN2, IN_2);
    if (input1->bitmap!=BITMAP(IN_1))
    {
        serialPrintf("ERROR: wrong bitmap in input1, expected %h, found %h\n",BITMAP(IN_1),input1->bitmap);
    }
    if (input2->bitmap!=BITMAP(IN_2))
    {
        serialPrintf("ERROR: wrong bitmap in input2, expected %h, found %h\n",BITMAP(IN_2),input2->bitmap);
    }

    input1->resetCalibrationData();
    input2->resetCalibrationData();
    Serial.println("-- GO --");
}

void loop()
{
    clutchValue_t value1,value2;
    bool autocal1, autocal2;
    bool changed1, changed2;

    input1->read(&value1,&changed1,&autocal1);
    input2->read(&value2,&changed2,&autocal2);
    if ((value1<value2-10) || (value1>value2+10)) {
        serialPrintf("ERROR: ADC readings should get close one to the other. ADC1: %d, ADC2: %d\n", value1, value2);
    }
    if (changed1 || autocal1 || changed2 || autocal2)
    {
        serialPrintf("Axis: %d ", value1);
        if (autocal1 || autocal2)
            Serial.println("(autocalibrated)");
        else
            Serial.println("");
    }

    vTaskDelay(DEBOUNCE_TICKS);
}

// void loop()
// {
//     clutchValue_t value;
//     bool autocal;
//     bool changed;
//     int min,max;

//     input1->read(&value, &changed, &autocal);
//     input1->getCalibrationData(&min,&max);
//     serialPrintf("Axis: %d, autocal: %d, changed: %d | Min: %d, Max:%d \n", value, autocal, changed,min,max);

//     vTaskDelay(DEBOUNCE_TICKS*10);
// }