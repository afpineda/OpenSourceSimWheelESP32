/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-05
 * @brief Integration Test. See [README](./README.md)
 * 
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 * 
 */

#include<Arduino.h>
#include"SimWheel.h"

//------------------------------------------------------
// Globals
//------------------------------------------------------

//------------------------------------------------------
// Arduino entry point
//------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    //while (!Serial);
    delay(2000);
    Serial.println("-- READY --");
    uiManager::begin();
    if (!ui::begin()) {
        Serial.println("OLED not detected!!");
        while(true);
    }
    Serial.println("-- GO --");
}

void loop()
{
    Serial.println("ui::showMenu");
    ui::showMenu("Menu","Option 1");
    delay(1000);
    ui::showInfo("Info","Unseen",SCR_FRAMESERVER_PRIORITY); // Should not show up

    delay(1000);
    ui::showMenu("Menu","Option 2");
    delay(6000); // No autohide
    ui::showMenu("Menu","Option 3");
    delay(500);
    ui::hideMenu(); // should hide
    delay(2000);

    Serial.println("ui::showInfo");
    ui::showInfo("Info","1234567890");
    uiManager::hide(SCR_FRAMESERVER_PRIORITY); // Should not hide
    delay(2000);
    ui::showInfo("Info","123456");
    delay(2000);
    uiManager::hide(SCR_INFO_PRIORITY); // should hide
    delay(2000);

    Serial.println("ui::showModal");
    ui::showModal("Modal 1", "Value 1"); 
    ui::showModal("Modal 2", "Value 2");
    ui::showMenu("Menu","Option 3");
    delay(1000);
    ui::showModal("Modal 3", "Value3",SCR_FRAMESERVER_PRIORITY); // should not show
    delay(1000);
    ui::showModal("Modal 4", "Value4",SCR_MENU_PRIORITY); // should show
    delay(1000);
    ui::hideMenu();
    delay(1000);
    ui::showLowBatteryNotice();
    delay(6000);
    uiManager::hide(SCR_INFO_PRIORITY); // should hide


    Serial.println("ui::showModal");
}

