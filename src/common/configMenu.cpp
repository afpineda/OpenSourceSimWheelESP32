/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Navigation through UI menus
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include "SimWheelTypes.h"
#include "SimWheel.h"
#include "strings.h"
#include <Preferences.h>

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

// NOTE: do not change order of enumerated items
typedef enum
{
    MENU_SELECT = 0,             /// Special meaning: confirm selection
    MENU_EXIT,                   /// Close config menu
    MENU_LANG,                   /// Choose language (submenu)
    MENU_FUNCTION,               /// Choose clutch paddles function (submenu)
    MENU_LOADPRESET,             /// Choose preset to load (submenu)
    MENU_SAVEPRESET,             /// Choose preset to save (submenu)
    MENU_EN,                     /// Select english
    MENU_ES,                     /// Select spanish
    MENU_FUNCTION_CLUTCH,        /// Select clutch function for clutch paddles
    MENU_FUNCTION_ALT,           /// Select alt function for clutch paddles
    MENU_FUNCTION_BUTTON,        /// Select bare button function for clutch paddles
    MENU_LOADPRESET_NUMBER,      /// Choose preset slot to load (submenu)
    MENU_SAVEPRESET_NUMBER,      /// Choose preset slot to save (submenu)
    MENU_LOADPRESET_NUMBER_UP,   /// Increase slot to load
    MENU_LOADPRESET_NUMBER_DOWN, /// Decrease slot to load
    MENU_SAVEPRESET_NUMBER_UP,   /// Increase slot to save
    MENU_SAVEPRESET_NUMBER_DOWN, /// Decrease slot to save
    MENU_ALT_FUNCTION,           /// Choose function of ALT buttons (submenu)
    MENU_ALT_FUNCTION_ALT,       /// Select alt function for ALT buttons
    MENU_ALT_FUNCTION_BUTTON,    /// Select bare button function for ALT buttons
    MENU_BATTERY,                /// Battery level (submenu)
    MENU_BATTERY_SHOW,           /// Show battery level
    MENU_POWEROFF,               /// Power off (submenu)
    MENU_BATTERY_RECALIBRATE,    /// Restart auto-calibration
    MENU_FRAMESERVER,            /// Frameserver (submenu)
    MENU_FRAMESERVER_ON,         /// Activate Frameserver
    MENU_FRAMESERVER_OFF         /// Deactivate Frameserver
} menu_t;

typedef enum
{
    NAV_PREV = 0,
    NAV_NEXT,
    NAV_ENTER,
    NAV_LEAVE
} nav_menu_t;

/*
  ###########  navigation paths #############
- EXIT
- LANG
    - (EN)
    - (ES)
- CLUTCH FUNCTION
    - (CLUTCH)
    - (ALT)
    - (BUTTON)
- ALT FUNCTION
    - (ALT)
    - (BUTTON)
- LOADPRESET
    - PRESET NUMBER
        -(UP)
        -(DOWN)
- SAVEPRESET
    - PRESET NUMBER
        -(UP)
        -(DOWN)
- Battery level
   - (Show battery level)
   - (Recalibrate)
- Power off
- Frame server
   - ON
   - OFF
*/

// NOTE: do not change order of array items
static const menu_t menuNav[27][4] =
    {
        // MENU_SELECT
        {MENU_SELECT, MENU_SELECT, MENU_SELECT, MENU_SELECT},
        // MENU_EXIT
        {MENU_FRAMESERVER, MENU_LANG, MENU_SELECT, MENU_EXIT},
        // MENU_LANG
        {MENU_EXIT, MENU_FUNCTION, MENU_EN, MENU_EXIT},
        // MENU_FUNCTION
        {MENU_LANG, MENU_ALT_FUNCTION, MENU_FUNCTION_CLUTCH, MENU_EXIT},
        // MENU_LOADPRESET
        {MENU_ALT_FUNCTION, MENU_SAVEPRESET, MENU_LOADPRESET_NUMBER, MENU_EXIT},
        // MENU_SAVEPRESET
        {MENU_LOADPRESET, MENU_BATTERY, MENU_SAVEPRESET_NUMBER, MENU_EXIT},
        // MENU_EN
        {MENU_ES, MENU_ES, MENU_SELECT, MENU_LANG},
        // MENU_ES
        {MENU_EN, MENU_EN, MENU_SELECT, MENU_LANG},
        // MENU_FUNCTION_CLUTCH
        {MENU_FUNCTION_BUTTON, MENU_FUNCTION_ALT, MENU_SELECT, MENU_FUNCTION},
        // MENU_FUNCTION_ALT
        {MENU_FUNCTION_CLUTCH, MENU_FUNCTION_BUTTON, MENU_SELECT, MENU_FUNCTION},
        // MENU_FUNCTION_BUTTON
        {MENU_FUNCTION_ALT, MENU_FUNCTION_CLUTCH, MENU_SELECT, MENU_FUNCTION},
        // MENU_LOADPRESET_NUMBER
        {MENU_LOADPRESET_NUMBER_DOWN, MENU_LOADPRESET_NUMBER_UP, MENU_SELECT, MENU_LOADPRESET},
        // MENU_SAVEPRESET_NUMBER
        {MENU_SAVEPRESET_NUMBER_DOWN, MENU_SAVEPRESET_NUMBER_UP, MENU_SELECT, MENU_SAVEPRESET},
        // MENU_LOADPRESET_NUMBER_UP
        {MENU_SELECT, MENU_SELECT, MENU_SELECT, MENU_SELECT},
        // MENU_LOADPRESET_NUMBER_DOWN
        {MENU_SELECT, MENU_SELECT, MENU_SELECT, MENU_SELECT},
        // MENU_SAVEPRESET_NUMBER_UP
        {MENU_SELECT, MENU_SELECT, MENU_SELECT, MENU_SELECT},
        // MENU_SAVEPRESET_NUMBER_DOWN
        {MENU_SELECT, MENU_SELECT, MENU_SELECT, MENU_SELECT},
        // MENU_ALT_FUNCTION
        {MENU_FUNCTION, MENU_LOADPRESET, MENU_ALT_FUNCTION_ALT, MENU_EXIT},
        // MENU_ALT_FUNCTION_ALT
        {MENU_ALT_FUNCTION_BUTTON, MENU_ALT_FUNCTION_BUTTON, MENU_SELECT, MENU_ALT_FUNCTION},
        // MENU_ALT_FUNCTION_BUTTON
        {MENU_ALT_FUNCTION_ALT, MENU_ALT_FUNCTION_ALT, MENU_SELECT, MENU_ALT_FUNCTION},
        // MENU_BATTERY
        {MENU_SAVEPRESET, MENU_POWEROFF, MENU_BATTERY_SHOW, MENU_EXIT},
        // MENU_BATTERY_SHOW
        {MENU_BATTERY_RECALIBRATE, MENU_BATTERY_RECALIBRATE, MENU_BATTERY_SHOW, MENU_BATTERY},
        // MENU_POWEROFF
        {MENU_BATTERY, MENU_FRAMESERVER, MENU_SELECT, MENU_EXIT},
        // MENU_BATTERY_RECALIBRATE
        {MENU_BATTERY_SHOW, MENU_BATTERY_SHOW, MENU_SELECT, MENU_BATTERY},
        // MENU_FRAMESERVER
        {MENU_POWEROFF, MENU_EXIT, MENU_FRAMESERVER_ON, MENU_EXIT},
        // MENU_FRAMESERVER_ON
        {MENU_FRAMESERVER_OFF, MENU_FRAMESERVER_OFF, MENU_SELECT, MENU_FRAMESERVER},
        // MENU_FRAMESERVER_OFF
        {MENU_FRAMESERVER_ON, MENU_FRAMESERVER_ON, MENU_SELECT, MENU_FRAMESERVER}};

static menu_t currentMenu = MENU_EXIT;
static bool visible = false;
#define MAX_PRESETS 40
static uint8_t presetSlot = 1; // NOTE: in range 1..MAX_PRESETS

#define CONFIRMATION_DELAY 1000 / portTICK_RATE_MS

static inputBitmap_t prevBitmap = 0;
static inputBitmap_t nextBitmap = 0;
static inputBitmap_t enterBitmap = 0;
static inputBitmap_t leaveBitmap = 0;

#define PREFS_NAMESPACE "configMenu"
#define KEY_CLUTCH_FUNCTION "f%02.2d"
#define KEY_CLUTCH_CALIBRATION "c%02.2d"
#define KEY_ALT_FUNCTION "a%02.2d"

// ----------------------------------------------------------------------------
// Execute actions
// ----------------------------------------------------------------------------

bool actionLoadPreset()
{
    Preferences prefs;
    if (prefs.begin(PREFS_NAMESPACE, true))
    {
        char key1[5];
        char key2[5];
        char key3[5];
        snprintf(key1, 5, KEY_CLUTCH_FUNCTION, presetSlot);
        snprintf(key2, 5, KEY_CLUTCH_CALIBRATION, presetSlot);
        snprintf(key3, 5, KEY_ALT_FUNCTION, presetSlot);
        if (!prefs.isKey(key1) || !prefs.isKey(key2) || !prefs.isKey(key3))
        {
            prefs.end();
            return false;
        }

        uint8_t value1 = prefs.getUChar(key1, (uint8_t)inputHub::getClutchFunction());
        int8_t value2 = prefs.getChar(key2, (int8_t)inputHub::getClutchBitePoint());
        bool value3 = prefs.getBool(key3, inputHub::getALTFunction());
        prefs.end();
        if ((value1 < CF_CLUTCH) || (value1 > CF_BUTTON)) //|| (value2 < CLUTCH_NONE_VALUE) || (value2 > CLUTCH_FULL_VALUE))
        {
            return false;
        }
        inputHub::setClutchFunction((clutchFunction_t)value1, true);
        inputHub::setClutchBitePoint((clutchValue_t)value2, true);
        inputHub::setALTFunction(value3);
        return true;
    }
    return false;
}

bool actionSavePreset()
{
    Preferences prefs;
    if (prefs.begin(PREFS_NAMESPACE, false))
    {
        char key1[5];
        char key2[5];
        char key3[5];
        snprintf(key1, 5, KEY_CLUTCH_FUNCTION, presetSlot);
        snprintf(key2, 5, KEY_CLUTCH_CALIBRATION, presetSlot);
        snprintf(key3, 5, KEY_ALT_FUNCTION, presetSlot);
        prefs.putUChar(key1, (uint8_t)inputHub::getClutchFunction());
        prefs.putChar(key2, (uint8_t)inputHub::getClutchBitePoint());
        prefs.putBool(key3, inputHub::getALTFunction());
        prefs.end();
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------
// Show menu
// ----------------------------------------------------------------------------

void showCurrentMenu()
{
    char aux[17];
    switch (currentMenu)
    {
    case MENU_EXIT:
        ui::showMenu((char *)str_menu, str_exit);
        break;
    case MENU_LANG:
        ui::showMenu((char *)str_menu, str_lang);
        break;
    case MENU_FUNCTION:
        ui::showMenu((char *)str_menu, str_clutch_function);
        break;
    case MENU_LOADPRESET:
        ui::showMenu((char *)str_menu, str_load_preset);
        break;
    case MENU_SAVEPRESET:
        ui::showMenu((char *)str_menu, str_save_preset);
        break;
    case MENU_EN:
        ui::showMenu(str_lang, (char *)str_lang_en);
        break;
    case MENU_ES:
        ui::showMenu(str_lang, (char *)str_lang_es);
        break;
    case MENU_FUNCTION_CLUTCH:
        ui::showMenu(str_clutch_function, str_clutch_function_value[CF_CLUTCH]);
        break;
    case MENU_FUNCTION_ALT:
        ui::showMenu(str_clutch_function, str_clutch_function_value[CF_ALT]);
        break;
    case MENU_FUNCTION_BUTTON:
        ui::showMenu(str_clutch_function, str_clutch_function_value[CF_BUTTON]);
        break;
    case MENU_LOADPRESET_NUMBER:
        snprintf(aux, 17, str_slot, presetSlot);
        ui::showMenu(str_load_preset, aux);
        break;
    case MENU_SAVEPRESET_NUMBER:
        snprintf(aux, 17, str_slot, presetSlot);
        ui::showMenu(str_save_preset, aux);
        break;
    case MENU_ALT_FUNCTION:
        ui::showMenu((char *)str_menu, str_alt_function);
        break;
    case MENU_ALT_FUNCTION_ALT:
        ui::showMenu(str_alt_function, str_clutch_function_value[CF_ALT]);
        break;
    case MENU_ALT_FUNCTION_BUTTON:
        ui::showMenu(str_alt_function, str_clutch_function_value[CF_BUTTON]);
        break;
    case MENU_BATTERY:
        ui::showMenu((char *)str_menu, str_battery);
        break;
    case MENU_BATTERY_SHOW:
        snprintf(aux, 17, "%03.3d%%", power::getLastBatteryLevel());
        ui::showMenu((char *)str_battery, aux);
        break;
    case MENU_POWEROFF:
        ui::showMenu((char *)str_menu, str_powerOff);
        break;
    case MENU_BATTERY_RECALIBRATE:
        ui::showMenu((char *)str_battery, (char *)str_recalibrate);
        break;
    case MENU_FRAMESERVER:
        ui::showMenu((char *)str_menu, (char *)str_frameserver);
        break;
    case MENU_FRAMESERVER_ON:
        ui::showMenu((char *)str_frameserver, (char *)str_on);
        break;
    case MENU_FRAMESERVER_OFF:
        ui::showMenu((char *)str_frameserver, (char *)str_off);
        break;
    default:
        // Should not enter here
        currentMenu = MENU_EXIT;
        showCurrentMenu();
        break;
    }
}

// ----------------------------------------------------------------------------
// Navigate
// ----------------------------------------------------------------------------

void navigate(nav_menu_t action)
{
    char aux[17];
    menu_t prev = currentMenu;
    menu_t next = menuNav[prev][action];
    // Serial.print("current: ");
    // Serial.print(prev);
    // Serial.print("next: ");
    // Serial.print(next);
    // Serial.println("");

    if (next == MENU_SELECT)
    {
        // Execute action
        switch (prev)
        {
        case MENU_ES:
            language::setLanguage(language_t::LANG_ES);
            ui::showModal(str_lang, (char *)str_lang_es, SCR_MENU_PRIORITY);
            currentMenu = MENU_LANG;
            break;

        case MENU_EN:
            language::setLanguage(language_t::LANG_EN);
            ui::showModal(str_lang, (char *)str_lang_en, SCR_MENU_PRIORITY);
            currentMenu = MENU_LANG;
            break;

        case MENU_EXIT:
            ui::hideMenu();
            visible = false;
            inputHub::notifyMenuExit();
            return;

        case MENU_FUNCTION_CLUTCH:
        case MENU_FUNCTION_ALT:
        case MENU_FUNCTION_BUTTON:
            clutchFunction_t cf;
            if (prev == MENU_FUNCTION_CLUTCH)
                cf = CF_CLUTCH;
            else if (prev == MENU_FUNCTION_ALT)
                cf = CF_ALT;
            else
                cf = CF_BUTTON;
            inputHub::setClutchFunction(cf, true);
            ui::showModal(str_clutch_function, str_clutch_function_value[cf], SCR_MENU_PRIORITY);
            currentMenu = MENU_EXIT;
            break;

        case MENU_LOADPRESET_NUMBER:
            if (actionLoadPreset())
            {
                snprintf(aux, 17, str_slot, presetSlot);
                ui::showModal(str_load_preset, aux, SCR_MENU_PRIORITY);
                currentMenu = MENU_EXIT;
            }
            break;

        case MENU_SAVEPRESET_NUMBER:
            if (actionSavePreset())
            {
                snprintf(aux, 17, str_slot, presetSlot);
                ui::showModal(str_save_preset, aux, SCR_MENU_PRIORITY);
                currentMenu = MENU_EXIT;
            }
            break;

        case MENU_ALT_FUNCTION_ALT:
            inputHub::setALTFunction(true, true);
            ui::showModal(str_alt_function, str_clutch_function_value[CF_ALT], SCR_MENU_PRIORITY);
            currentMenu = MENU_EXIT;
            break;

        case MENU_ALT_FUNCTION_BUTTON:
            inputHub::setALTFunction(false, true);
            ui::showModal(str_alt_function, str_clutch_function_value[CF_BUTTON], SCR_MENU_PRIORITY);
            currentMenu = MENU_EXIT;
            break;

        case MENU_POWEROFF:
            power::powerOff(true);
            // Note: while testing the system could not get powered off
            currentMenu = MENU_EXIT;
            break;

        case MENU_BATTERY_RECALIBRATE:
            batteryCalibration::restartAutoCalibration();
            ui::showModal((char *)str_battery, (char *)str_recalibrate, SCR_MENU_PRIORITY);
            currentMenu = MENU_EXIT;
            break;

        case MENU_FRAMESERVER_ON:
            ui::frameServerSetEnabled(true);
            ui::showModal((char *)str_frameserver, (char *)str_on, SCR_MENU_PRIORITY);
            currentMenu = MENU_FRAMESERVER;
            break;

        case MENU_FRAMESERVER_OFF:
            ui::frameServerSetEnabled(false);
            ui::showModal((char *)str_frameserver, (char *)str_off, SCR_MENU_PRIORITY);
            currentMenu = MENU_FRAMESERVER;
            break;

        default:
            // Should not enter here
            currentMenu = MENU_EXIT;
            break;
        }
    }
    else
    {
        // navigate
        switch (next)
        {
        case MENU_FUNCTION:
            currentMenu = next;
            if (!inputHub::hasClutchPaddles())
            {
                navigate(action);
                return;
            }
            break;
        case MENU_ALT_FUNCTION:
            currentMenu = next;
            if (!inputHub::hasALTButtons())
            {
                navigate(action);
                return;
            }
            break;
        case MENU_LOADPRESET_NUMBER_UP:
            if (presetSlot < MAX_PRESETS)
                presetSlot++;
            currentMenu = MENU_LOADPRESET_NUMBER;
            break;
        case MENU_LOADPRESET_NUMBER_DOWN:
            if (presetSlot > 1)
                presetSlot--;
            currentMenu = MENU_LOADPRESET_NUMBER;
            break;
        case MENU_SAVEPRESET_NUMBER_UP:
            if (presetSlot < MAX_PRESETS)
                presetSlot++;
            currentMenu = MENU_SAVEPRESET_NUMBER;
            break;
        case MENU_SAVEPRESET_NUMBER_DOWN:
            if (presetSlot > 1)
                presetSlot--;
            currentMenu = MENU_SAVEPRESET_NUMBER;
            break;
        case MENU_BATTERY:
            currentMenu = next;
            if (!power::hasBatteryMonitor())
            {
                navigate(action);
                return;
            }
        default:
            currentMenu = next;
            break;
        }
    }
    showCurrentMenu();
}

// ----------------------------------------------------------------------------
// Enter/exit menu
// ----------------------------------------------------------------------------

bool configMenu::toggle()
{
    if (ui::isAvailable())
    {
        visible = !visible;
        if (visible)
        {
            currentMenu = MENU_EXIT;
            showCurrentMenu();
        }
        else
        {
            ui::hideMenu();
        }
        return visible;
    }
    return false;
}

// ----------------------------------------------------------------------------
// Navigation
// ----------------------------------------------------------------------------

void configMenu::onInput(inputBitmap_t globalState, inputBitmap_t changes)
{
    // Note: action is triggered on button release
    // Serial.print("onInput: ");
    // Serial.print(globalState);
    // Serial.print(" prev: ");
    // Serial.println(prevBitmap);
    if ((changes & prevBitmap) && !(globalState & prevBitmap))
        navigate(NAV_PREV);
    else if ((changes & nextBitmap) && !(globalState & nextBitmap))
        navigate(NAV_NEXT);
    else if ((changes & enterBitmap) && !(globalState & enterBitmap))
        navigate(NAV_ENTER);
    else if ((changes & leaveBitmap) && !(globalState & leaveBitmap))
        navigate(NAV_LEAVE);
    // else
    //     Serial.println("No input");
}

void configMenu::setNavButtons(
    inputNumber_t prevButtonNumber,
    inputNumber_t nextButtonNumber,
    inputNumber_t selectButtonNumber,
    inputNumber_t cancelButtonNumber)
{
    prevBitmap = BITMAP(prevButtonNumber);
    nextBitmap = BITMAP(nextButtonNumber);
    enterBitmap = BITMAP(selectButtonNumber);
    leaveBitmap = BITMAP(cancelButtonNumber);
}