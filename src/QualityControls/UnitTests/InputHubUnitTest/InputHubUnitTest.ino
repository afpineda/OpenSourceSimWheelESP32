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

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

#define CLUTCHV_TO_STORE -3
#define FUNCTION_TO_STORE CF_ALT
#define LCLUTCH 1
#define RCLUTCH 2
#define ALT 3
#define MENU 4
#define OTHER 5
#define UP 6
#define DOWN 7
#define LEFT 8
#define RIGHT 9

#define LCLUTCH_B BITMAP(LCLUTCH)
#define RCLUTCH_B BITMAP(RCLUTCH)
#define ALT_B BITMAP(ALT)
#define MENU_B BITMAP(MENU)
#define OTHER_B BITMAP(OTHER)
#define UP_B BITMAP(UP)
#define DOWN_B BITMAP(DOWN)
#define LEFT_B BITMAP(LEFT)
#define RIGHT_B BITMAP(RIGHT)

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

void configMenu::onInput(inputBitmap_t globalState, inputBitmap_t changes)
{
    if (changes)
    {
        Serial.print("MENU: ");
        Serial.print(globalState);
        Serial.print(";");
        Serial.println(changes);
    }
}

void hidImplementation::reset()
{
}

void hidImplementation::reportInput(
    inputBitmap_t globalState,
    bool altEnabled,
    uint8_t POVstate)
{
    Serial.print("INPUT: ");
    Serial.print(globalState);
    if (altEnabled)
        Serial.print(" ALT");
    Serial.print(" Clutch: ");
    Serial.print(clutchValue);
    Serial.print(" POV ");
    Serial.print(POVstate);
    Serial.println("");
}

void ui::showBitePoint(clutchValue_t value)
{
    Serial.print("Bite point: ");
    Serial.println(value);
}

void ui::showSaveNote()
{
}

bool configMenu::toggle()
{
    Serial.println("**menu button**");
    return true;
}

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

void pushMenuButton()
{
    inputHub::onStateChanged(MENU_B, MENU_B);
    delay(2200);
    inputHub::onStateChanged(0, MENU_B);
}

void pressAndRelease(inputBitmap_t bmp)
{
    inputHub::onStateChanged(bmp, bmp);
    inputHub::onStateChanged(0, bmp);
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    clutchValue_t clutch;
    clutchFunction_t f;
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("-- READY --");
    inputHub::begin();
    inputHub::setClutchPaddles(LCLUTCH, RCLUTCH);
    inputHub::setALTButton(ALT);
    inputHub::setMenuButton(MENU);
    inputHub::setDPADControls(UP, DOWN, LEFT, RIGHT);
    Serial.println("-- GO --");

    Serial.println("- stored preferences -");
    clutch = inputHub::getClutchBitePoint();
    if (clutch != CLUTCHV_TO_STORE)
        Serial.println("ERROR: bite point not stored");
    else
        Serial.println("bite point OK");
    f = inputHub::getClutchFunction();
    if (f != FUNCTION_TO_STORE)
        Serial.println("ERROR: clutch function not stored");
    else
        Serial.println("clutch function OK");
    if (inputHub::getALTFunction())
        Serial.println("ERROR: ALT function not stored");
    else
        Serial.println("ALT function OK");

    Serial.println("- simulate POV operation (valid input) -");
    pressAndRelease(UP_B); // POV = 1
    pressAndRelease(DOWN_B); // POV = 5
    pressAndRelease(LEFT_B); // POV = 7
    pressAndRelease(RIGHT_B); // POV = 3
    pressAndRelease(UP_B | LEFT_B); // POV = 8
    pressAndRelease(DOWN_B | LEFT_B); // POV = 6
    pressAndRelease(UP_B | RIGHT_B); // POV = 2
    pressAndRelease(DOWN_B | RIGHT_B); // POV = 4
    Serial.println("- simulate POV operation (invalid input) -");
    pressAndRelease(UP_B | DOWN_B);
    pressAndRelease(LEFT_B | RIGHT_B);
    Serial.println("- simulate POV operation while ALT pressed -");
    inputHub::setALTFunction(true,false);
    pressAndRelease(UP_B | ALT_B);
    pressAndRelease(DOWN_B | ALT_B);
    pressAndRelease(LEFT_B | ALT_B);
    pressAndRelease(RIGHT_B | ALT_B);

    Serial.println("- simulate ALT operation -");
    inputHub::setALTFunction(true, false);
    pressAndRelease(ALT_B);
    inputHub::setALTFunction(false, false);
    pressAndRelease(ALT_B);
    inputHub::setALTFunction(true, false);
    pressAndRelease(ALT_B | OTHER_B);

    Serial.println("- simulate clutch operation (clutch function) -");
    inputHub::setClutchFunction(CF_CLUTCH);
    pressAndRelease(LCLUTCH_B);
    pressAndRelease(RCLUTCH_B);
    pressAndRelease(LCLUTCH_B | RCLUTCH_B);

    Serial.println("- simulate clutch operation (ALT function) -");
    inputHub::setClutchFunction(CF_ALT);
    pressAndRelease(LCLUTCH_B);
    pressAndRelease(RCLUTCH_B);
    pressAndRelease(LCLUTCH_B | RCLUTCH_B);

    Serial.println("- simulate clutch operation (BUTTON function) -");
    inputHub::setClutchFunction(CF_BUTTON);
    pressAndRelease(LCLUTCH_B);
    pressAndRelease(RCLUTCH_B);

    Serial.println("- simulate config menu -");
    pushMenuButton();
    pressAndRelease(ALT_B);
    pushMenuButton();

    Serial.println("- save preferences -");
    inputHub::setClutchFunction(FUNCTION_TO_STORE);
    inputHub::setClutchBitePoint(CLUTCHV_TO_STORE, true);
    inputHub::setALTFunction(false, true);
    for (int i = 0; i < 30; i++)
        delay(1000);

    Serial.println("-- END --");
    for (;;)
        ;
}

void loop()
{
}