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
#include "SimWheelTypes.h"
#include "debugUtils.h"
#include "strings.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

#define PREV 0
#define NEXT 1
#define SELECT 2
#define CANCEL 3

int prevBmp, nextBmp, selectBmp, cancelBmp;

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

extern void ui_assertEquals(char *title, char *info);
extern void ui_assertModalEquals(char *title, char *info);

int power::getLastBatteryLevel()
{
    return 98;
}

void power::powerOff(bool forced)
{
}

bool power::hasBatteryMonitor()
{
    return true;
}

void batteryCalibration::restartAutoCalibration()
{
}

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    int testCount = 1;
    Serial.begin(115200);
    while (!Serial)
        ;

    Serial.println("-- READY --");
    configMenu::setNavButtons(PREV, NEXT, SELECT, CANCEL);
    prevBmp = BITMAP(PREV);
    nextBmp = BITMAP(NEXT);
    selectBmp = BITMAP(SELECT);
    cancelBmp = BITMAP(CANCEL);
    Serial.println("-- GO --");

    // Show menu
    printTestHeader(testCount++); // 1
    configMenu::toggle();         // visible
    ui_assertEquals((char *)str_menu, str_exit);

    // Navigate forward
    printTestHeader(testCount++); // 2
    configMenu::onInput(0, nextBmp);
    configMenu::onInput(0, nextBmp);
    configMenu::onInput(0, nextBmp);
    ui_assertEquals((char *)str_menu, str_alt_function);

    // hide menu
    printTestHeader(testCount++); // 3
    configMenu::toggle();         // not visible
    ui_assertEquals((char *)"", (char *)"");

    // Navigate backwards
    printTestHeader(testCount++); // 4
    configMenu::toggle();         // visible
    configMenu::onInput(0, prevBmp);
    configMenu::onInput(0, prevBmp);
    configMenu::onInput(0, prevBmp);
    configMenu::onInput(0, prevBmp);
    configMenu::onInput(0, prevBmp);
    configMenu::onInput(0, prevBmp);
    configMenu::onInput(0, prevBmp);
    ui_assertEquals((char *)str_menu, str_clutch_function);

    // Select function for clutch paddles
    printTestHeader(testCount++); // 5
    configMenu::onInput(0, selectBmp);
    ui_assertEquals((char *)str_clutch_function, str_clutch_function_value[CF_CLUTCH]);
    printTestHeader(testCount++);
    configMenu::onInput(0, nextBmp);
    ui_assertEquals((char *)str_clutch_function, str_clutch_function_value[CF_ALT]);
    printTestHeader(testCount++);
    configMenu::onInput(0, prevBmp);
    configMenu::onInput(0, prevBmp);
    ui_assertEquals((char *)str_clutch_function, str_clutch_function_value[CF_BUTTON]);

    // Select button function for clutch paddles
    printTestHeader(testCount++); // 8
    configMenu::onInput(0, selectBmp);
    ui_assertModalEquals((char *)str_clutch_function, str_clutch_function_value[CF_BUTTON]);
    ui_assertEquals((char *)str_menu, str_exit);

    // Nav to clutch paddle function and back to exit
    configMenu::toggle();         // not visible
    printTestHeader(testCount++); // 9
    configMenu::toggle();         // visible
    configMenu::onInput(0, nextBmp);
    configMenu::onInput(0, nextBmp);
    configMenu::onInput(0, selectBmp);
    configMenu::onInput(0, nextBmp);
    configMenu::onInput(0, nextBmp);
    configMenu::onInput(0, cancelBmp);
    configMenu::onInput(0, cancelBmp);
    configMenu::onInput(0, selectBmp); // EXIT Selected. Not visible
    ui_assertEquals((char *)"", (char *)"");
    if (!configMenu::toggle())
    {
        Serial.println("MENU WAS EXPECTED TO BE HIDDEN");
    }

    // Save preset 3
    printTestHeader(testCount++); // 10
    configMenu::onInput(0, prevBmp);
    configMenu::onInput(0, prevBmp);
    configMenu::onInput(0, prevBmp);
    configMenu::onInput(0, prevBmp);
    configMenu::onInput(0, selectBmp);
    configMenu::onInput(0, nextBmp);
    configMenu::onInput(0, nextBmp);
    configMenu::onInput(0, selectBmp);
    ui_assertModalEquals(str_save_preset, (char *)"Slot 03");

    // Select function for ALT buttons
    configMenu::toggle();         // not visible
    printTestHeader(testCount++); // 11
    configMenu::toggle();         // visible
    configMenu::onInput(0, nextBmp);
    configMenu::onInput(0, nextBmp);
    configMenu::onInput(0, nextBmp);
    configMenu::onInput(0, selectBmp);
    ui_assertEquals(str_alt_function, str_clutch_function_value[CF_ALT]);
    configMenu::onInput(0, prevBmp);
    configMenu::onInput(0, selectBmp);
    ui_assertModalEquals(str_alt_function, str_clutch_function_value[CF_BUTTON]);

    // battery level menu
    configMenu::toggle();         // not visible
    printTestHeader(testCount++); // 12
    configMenu::toggle();         // visible
    configMenu::onInput(0, prevBmp);
    configMenu::onInput(0, prevBmp);
    configMenu::onInput(0, prevBmp);
    configMenu::onInput(0, selectBmp);
    ui_assertEquals(str_battery, (char *)"098%");
    configMenu::onInput(0, selectBmp);
    ui_assertEquals(str_battery, (char *)"098%");
    configMenu::onInput(0, prevBmp);
    configMenu::onInput(0, nextBmp);
    ui_assertEquals(str_battery, (char *)"098%");
    configMenu::onInput(0, prevBmp);
    ui_assertEquals(str_battery, str_recalibrate);
    configMenu::onInput(0, prevBmp);
    configMenu::onInput(0, nextBmp);
    ui_assertEquals(str_battery, str_recalibrate);
    configMenu::onInput(0, prevBmp);
    ui_assertEquals(str_battery, (char *)"098%");

    // Frameserver menu
    configMenu::toggle();         // not visible
    printTestHeader(testCount++); // 13
    configMenu::toggle();         // visible
    configMenu::onInput(0, prevBmp);
    ui_assertEquals(str_menu, str_frameserver);
    configMenu::onInput(0, selectBmp);
    ui_assertEquals(str_frameserver, str_on);
    configMenu::onInput(0, nextBmp);
    ui_assertEquals(str_frameserver, str_off);
    configMenu::onInput(0, nextBmp);
    ui_assertEquals(str_frameserver, str_on);
    configMenu::onInput(0, selectBmp);
    ui_assertModalEquals(str_frameserver, str_on);

    // END
    Serial.println("-- END --");
    for (;;)
        ;
}

void loop()
{
    // Not executed
}