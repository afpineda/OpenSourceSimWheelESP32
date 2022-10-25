/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Unit Test. See [README](./README.md)
 * 
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 * 
 */

#include <Arduino.h>
#include "strings.h"
#include "SimWheel.h"

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

void printAllStrings()
{
    Serial.println(str_clutch_function);
    Serial.println(str_clutch_function_value[0]);
    Serial.println(str_clutch_function_value[1]);
    Serial.println(str_clutch_function_value[2]);
    Serial.println(str_clutch_cal);
    Serial.println(str_lang);
    Serial.println(str_slot);
    Serial.println(str_load_preset);
    Serial.println(str_save_preset);
    Serial.println(str_exit);
    Serial.println(str_saved);
    Serial.println(str_connected);
    Serial.println(str_alt_function);
    Serial.println(str_powerOff);
    Serial.println(str_battery);
    Serial.println(str_recalibrate);
    Serial.println(str_on);
    Serial.println(str_off);
    Serial.println(str_wellcome);

    Serial.println(str_lang_en);
    Serial.println(str_lang_es);
    Serial.println(str_menu);
    Serial.println(str_frameserver);
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("--START--");
    language::begin();

    language_t l = language::getLanguage();
    if (l == LANG_ES)
        Serial.println("Current language: OK");
    else
        Serial.println("Current language: ***WRONG***");
    
    Serial.println("--GO--");
    
    language::setLanguage(LANG_EN);
    l = language::getLanguage();
    if (l!=LANG_EN)
        Serial.println("**ERROR** Expected LANG_EN");
    printAllStrings();
    Serial.println("----");
    
    language::setLanguage(LANG_ES);
    l = language::getLanguage();
    if (l!=LANG_ES)
        Serial.println("**ERROR** Expected LANG_ES");
    printAllStrings();
    Serial.println("--END--");
    
    while (true)
        ;
}

void loop()
{
}