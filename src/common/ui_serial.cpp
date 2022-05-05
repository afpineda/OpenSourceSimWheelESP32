/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Implementation of the `ui` namespace through the USB serial port
 *        for testing porpouses
 * 
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 * 
 */

#include <Arduino.h>
#include "SimWheel.h"
#include "strings.h"

using namespace uiManager;

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

bool ui::begin()
{
    Serial.println("*UI INIT*");
    return true;
}

// ----------------------------------------------------------------------------
// Display
// ----------------------------------------------------------------------------

void ui::clear()
{
   Serial.println("***CLS***");
}

void ui::display(uint8_t *buffer)
{
    Serial.println("__Screen__");
    Serial.println((char*)buffer);
    Serial.println("__________");
}


// ----------------------------------------------------------------------------
// Paint
// ----------------------------------------------------------------------------

void ui::showBitePoint(clutchValue_t value)
{
    uint8_t *buffer = enterDisplay(SCR_MENU_PRIORITY);
    int iPercent = (value - CLUTCH_NONE_VALUE) * 100 / (CLUTCH_FULL_VALUE - CLUTCH_NONE_VALUE);
    snprintf((char *)buffer, 80, "Bite point: % 3d%%", iPercent);
    exitDisplay(SCR_MENU_PRIORITY, true);
}

void ui::showMenu(const char *title, const char *selection)
{
    uint8_t *buffer = enterDisplay(SCR_MENU_PRIORITY);
    snprintf((char *)buffer, 80, "MENU %s: %s", title,selection);
    exitDisplay(SCR_MENU_PRIORITY, false);
}

void ui::hideMenu()
{
    uiManager::hide(SCR_MENU_PRIORITY);
}

void ui::showInfo(const char *title, const char *info, screenPriority_t priority)
{
    uint8_t *buffer = enterDisplay(priority);
    snprintf((char *)buffer, 80, "INFO %s: %s", title,info);
    exitDisplay(priority, true);
}

void ui::showModal(const char *title, const char *info, screenPriority_t priority)
{
    uint8_t *buffer = enterDisplay(priority);
    snprintf((char *)buffer, 80, "MODAL %s: %s", title,info);
    exitDisplay(priority, false);
    vTaskDelay(2000);
    hide(priority);
}

void ui::showSaveNote()
{
    ui::showInfo(nullptr, str_saved, SCR_MENU_PRIORITY);
}

void ui::showConnectedNotice()
{
    ui::showInfo(nullptr, str_connected, SCR_INFO_PRIORITY);
}

void ui::showBLEDiscoveringNotice()
{
    ui::showInfo(nullptr, (const char *)"(((.)))", SCR_INFO_PRIORITY);
}

void ui::turnOff()
{
    Serial.println("*** DISPLAY OFF ***");
}

void ui::frameServerSetEnabled(bool state)
{

}

bool ui::isAvailable()
{
    return true;
}
