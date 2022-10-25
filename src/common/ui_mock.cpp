/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Dummy implementation of the `ui` namespace
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include "SimWheel.h"
#include <string.h>
#include <Arduino.h>

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

#define MAX_STR_SIZE 256
static char currentTitle[MAX_STR_SIZE];
static char currentInfo[MAX_STR_SIZE];
static char currentModalTitle[MAX_STR_SIZE];
static char currentModalInfo[MAX_STR_SIZE];

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

bool ui::begin(int pixels_width,
               int pixels_height,
               displayType_t displayType,
               bool flipUpsideDown)
{
  return true;
}

void ui::clear()
{
  memset(currentTitle, 0, MAX_STR_SIZE);
  memset(currentInfo, 0, MAX_STR_SIZE);
  memset(currentModalTitle, 0, MAX_STR_SIZE);
  memset(currentModalInfo, 0, MAX_STR_SIZE);
}

void ui::display(uint8_t *buffer)
{
}

void ui::turnOff()
{
}

void ui::showBitePoint(clutchValue_t value)
{
}

void ui::showMenu(const char *title, const char *selection)
{
  memset(currentTitle, 0, MAX_STR_SIZE);
  memset(currentInfo, 0, MAX_STR_SIZE);
  strncpy((char *)currentTitle, (char *)title, MAX_STR_SIZE);
  strncpy((char *)currentInfo, (char *)selection, MAX_STR_SIZE);
}

void ui::hideMenu()
{
  clear();
}

void ui::showInfo(const char *title, const char *info, screenPriority_t priority)
{
  showMenu(title, info);
}

void ui::showModal(const char *title, const char *info, screenPriority_t priority)
{
  memset(currentModalTitle, 0, MAX_STR_SIZE);
  memset(currentModalInfo, 0, MAX_STR_SIZE);
  strncpy((char *)currentModalTitle, (char *)title, MAX_STR_SIZE);
  strncpy((char *)currentModalInfo, (char *)info, MAX_STR_SIZE);
}

void ui::showSaveNote()
{
}

void ui::showConnectedNotice()
{
}

void ui::showBLEDiscoveringNotice()
{
}

void ui::showLowBatteryNotice()
{
}

bool ui::isAvailable()
{
  return true;
}

void ui::frameServerSetEnabled(bool enabled)
{
  
}

//------------------------------------------------------------------
// Testing
//------------------------------------------------------------------

void ui_assertEquals(char *title, char *info)
{
  if ((strcmp(currentTitle, title) != 0) || (strcmp(currentInfo, info) != 0))
  {
    Serial.println("UI MISMATCH:");
    Serial.print("EXPECTED: ");
    Serial.print(title);
    Serial.print(" / ");
    Serial.println(info);
    Serial.print("OBTAINED: ");
    Serial.print(currentTitle);
    Serial.print(" / ");
    Serial.println(currentInfo);
    Serial.println("----------------");
  }
}

void ui_assertModalEquals(char *title, char *info)
{
  if ((strcmp(currentModalTitle, title) != 0) || (strcmp(currentModalInfo, info) != 0))
  {
    Serial.println("UI MISMATCH AT MODAL:");
    Serial.print("EXPECTED: ");
    Serial.print(title);
    Serial.print(" / ");
    Serial.println(info);
    Serial.print("OBTAINED: ");
    Serial.print(currentModalTitle);
    Serial.print(" / ");
    Serial.println(currentModalInfo);
    Serial.println("----------------");
  }
}
