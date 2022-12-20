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
clutchValue_t last1 = CLUTCH_NONE_VALUE;
clutchValue_t last2 = CLUTCH_NONE_VALUE;

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("-- READY --");

    input1 = new AnalogAxisInput(TEST_ANALOG_PIN1, 1, nullptr);
    input2 = new AnalogAxisInput(TEST_ANALOG_PIN2, 2, input1);

    if ((input2->getNextInChain() != input1) || (input1->getNextInChain() != nullptr))
    {
        Serial.println("ERROR: chain failure");
        for(;;);
    }

    uint8_t index=99;
    input1->read(&index,&last1);
    if (index!=1) {
        serialPrintf("ERROR: wrong index, expected %d, found %d\n",1,index);
        for(;;);
    }
    input2->read(&index,&last2);
    if (index!=2) {
        serialPrintf("ERROR: wrong index, expected %d, found %d\n",2,index);
        for(;;);
    }

    Serial.println("-- GO --");
}

void loop()
{
    uint8_t index=99;
    clutchValue_t v1,v2;
    input1->read(&index,&v1);
    input2->read(&index,&v2);
    if ((v1!=last1) || (v2!=last2)) {
        serialPrintf("ADC1: %d, ADC2: %d\n",v1,v2);
        last1=v1;
        last2=v2;
    }
    
    vTaskDelay(DEBOUNCE_TICKS);
}