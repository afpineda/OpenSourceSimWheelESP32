/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Unit Test. See [README](./README.md)
 * 
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 * 
 */

#include <Arduino.h>
#include "SimWheel.h"

//------------------------------------------------------
// mocks
//------------------------------------------------------

void ui::clear()
{
    Serial.println("***CLS***");
}

void ui::display(uint8_t *buffer)
{
    Serial.println((char *)buffer);
}

//------------------------------------------------------
// macro
//------------------------------------------------------

void macro(const char *msg, screenPriority_t priority, bool autohide = false)
{
    char *buffer = (char *)uiManager::enterDisplay(priority);
    memset(buffer,0,64);
    strncpy(buffer, msg, 63);
    uiManager::exitDisplay(priority, autohide);
}

//------------------------------------------------------
// Arduino entry point
//------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("--READY--");
    uiManager::begin();
    Serial.println("--GO--");
}

void loop()
{
    Serial.println("#1");
    Serial.println("-- show and then hide");
    macro("info", SCR_INFO_PRIORITY);
    uiManager::hide(SCR_INFO_PRIORITY);
    macro("menu", SCR_MENU_PRIORITY);
    uiManager::hide(SCR_MENU_PRIORITY);
    macro("comm", SCR_COMM_PRIORITY);
    uiManager::hide(SCR_COMM_PRIORITY);

    Serial.println("#2");
    Serial.println("-- increasing priority show");
    macro("comm", SCR_COMM_PRIORITY);
    macro("menu", SCR_MENU_PRIORITY);
    macro("info", SCR_INFO_PRIORITY);

    Serial.println("-- decreasing priority hide");
    uiManager::hide(SCR_INFO_PRIORITY);
    uiManager::hide(SCR_MENU_PRIORITY);
    uiManager::hide(SCR_COMM_PRIORITY);

    Serial.println("#3");
    Serial.println("-- decreasing priority show");
    macro("info", SCR_INFO_PRIORITY);
    macro("menu", SCR_MENU_PRIORITY);
    macro("comm", SCR_COMM_PRIORITY);

    Serial.println("-- decreasing priority hide");
    uiManager::hide(SCR_INFO_PRIORITY);
    uiManager::hide(SCR_MENU_PRIORITY);
    uiManager::hide(SCR_COMM_PRIORITY);

    Serial.println("#4");
    Serial.println("-- decreasing priority show");
    macro("info", SCR_INFO_PRIORITY);
    macro("menu", SCR_MENU_PRIORITY);
    macro("comm", SCR_COMM_PRIORITY);

    Serial.println("-- increasing priority hide");
    uiManager::hide(SCR_COMM_PRIORITY);
    uiManager::hide(SCR_MENU_PRIORITY);
    uiManager::hide(SCR_INFO_PRIORITY);

    Serial.println("#5");
    Serial.println("-- increasing priority show with autohide");
    macro("comm", SCR_COMM_PRIORITY, true);
    macro("menu", SCR_MENU_PRIORITY, true);
    macro("info", SCR_INFO_PRIORITY, true);
    delay((DEFAULT_UI_TIME_us/1000)+500);

    Serial.println("#6");
    Serial.println("-- autohide non visible screen");
    macro("info", SCR_INFO_PRIORITY,false);
    macro("comm", SCR_COMM_PRIORITY,true);
    delay((DEFAULT_UI_TIME_us/1000)+500);
    uiManager::hide(SCR_INFO_PRIORITY);

    Serial.println("-- END --");
    for (;;)
        ;
}