/**
 * @file UserSettingsTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-01-13
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.h"
#include "SimWheelTypes.h"
#include "HardwareSerial.h"
#include "nvs_flash.h"

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

void inputs::update()
{
}

void notify::bitePoint(clutchValue_t bp)
{
}

void hidImplementation::reportChangeInConfig()
{
}

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

void printUserSettings(bool printMap = true)
{
    Serial.printf("   Bite point: %u\n", userSettings::bitePoint);
    Serial.printf("   Clutch working mode: %u\n", userSettings::cpWorkingMode);
    Serial.printf("   ALT buttons working mode: %u\n", userSettings::altButtonsWorkingMode);
    Serial.printf("   DPAD working mode: %u\n", userSettings::dpadWorkingMode);
    Serial.printf("   Security lock: %d\n", userSettings::securityLock);
    if (printMap)
    {
        Serial.printf("   Button 0 map: %hhu %hhu\n", userSettings::buttonsMap[0][0], userSettings::buttonsMap[1][0]);
        Serial.printf("   Button 63 map: %hhu %hhu\n", userSettings::buttonsMap[0][63], userSettings::buttonsMap[1][63]);
    }
}

void printTestCase()
{
    Serial.println("");
    Serial.println("-- The following user settings has been saved to flash memory --");
    printUserSettings();
    Serial.println("Take note then RESET.");
    Serial.println("----------------------------------------------------------------");
}

void printTestCaseAutoSave()
{
    Serial.println("");
    Serial.println("-- AUTOSAVE: The following user settings are about to be saved to flash memory --");
    printUserSettings(false);
    Serial.println("Please, WAIT for 40 seconds or so...");
    delay(40000);
    Serial.println("Take note then RESET.");
    Serial.println("----------------------------------------------------------------");
}

void setTestCase1()
{
    userSettings::setBitePoint(CLUTCH_DEFAULT_VALUE);
    userSettings::setCPWorkingMode(CF_CLUTCH);
    userSettings::setALTButtonsWorkingMode(true);
    userSettings::setDPADWorkingMode(true);
    userSettings::resetButtonsMap();
    userSettings::setSecurityLock(false);
    userSettings::saveNow();
    printTestCase();
}

void setTestCase2()
{
    userSettings::setBitePoint(CLUTCH_1_4_VALUE);
    userSettings::setCPWorkingMode(CF_AXIS);
    userSettings::setALTButtonsWorkingMode(false);
    userSettings::setDPADWorkingMode(false);
    userSettings::resetButtonsMap();
    userSettings::setButtonMap(0, 120, 121);
    userSettings::setButtonMap(63, 121, 120);
    userSettings::setSecurityLock(true);
    userSettings::saveNow();
    printTestCase();
}

void setTestCase3()
{
    userSettings::setBitePoint(CLUTCH_3_4_VALUE);
    userSettings::setCPWorkingMode(CF_ALT);
    userSettings::setALTButtonsWorkingMode(true);
    userSettings::setDPADWorkingMode(false);
    userSettings::resetButtonsMap();
    userSettings::setButtonMap(0, 15, 25);
    userSettings::setButtonMap(63, 25, 15);
    userSettings::setSecurityLock(false);
    userSettings::saveNow();
    printTestCase();
}

void setTestCase4()
{
    userSettings::setBitePoint(CLUTCH_NONE_VALUE);
    userSettings::setCPWorkingMode(CF_BUTTON);
    userSettings::setALTButtonsWorkingMode(true);
    userSettings::setDPADWorkingMode(false);
    userSettings::resetButtonsMap();
    userSettings::setSecurityLock(true);
    printTestCaseAutoSave();
}

void cleanMrProper()
{
    nvs_flash_erase();
    Serial.println("** The NVS flash partition has been erased **");
    Serial.println("");
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    Serial.println("-- Stored user settings follows --");
    Serial.println("-- Check that they match the selected test case --");
    userSettings::begin();
    printUserSettings();

    Serial.println("");
}

//------------------------------------------------------------------

void loop()
{
    Serial.println("-- Select another test case --");
    Serial.println("(test case will be printed later)");
    Serial.println("1 = factory defaults");
    Serial.println("2 = test case");
    Serial.println("3 = another test case");
    Serial.println("4 = autosave test case");
    Serial.println("X = ERASE NVS flash partition (CAUTION: will erase everything)");
    Serial.println("Type test number and press ENTER...");
    while (!Serial.available())
        ;
    int c = Serial.peek();
    if (c == '1')
    {
        setTestCase1();
    }
    else if (c == '2')
    {
        setTestCase2();
    }
    else if (c == '3')
    {
        setTestCase3();
    }
    else if (c == '4')
    {
        setTestCase4();
    }
    else if ((c == 'X') || (c == 'x'))
    {
        cleanMrProper();
    }
    while (Serial.read() >= 0)
        ;
}