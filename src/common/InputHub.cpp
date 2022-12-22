/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Implementation of the `inputHub` namespace
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include "SimWheel.h"
// #include <FreeRTOS.h>
#include <Preferences.h>

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

using namespace inputHub;

// Related to ALT buttons
static inputBitmap_t altBitmap = 0;
static bool currentALTFunction = true;

// Related to clutch paddles
static clutchFunction_t currentClutchFunction = CF_CLUTCH;
static inputBitmap_t leftClutchBitmap = 0;
static inputBitmap_t rightClutchBitmap = 0;
static inputBitmap_t calibrateUpBitmap = 0;
static inputBitmap_t calibrateDownBitmap = 0;
static clutchValue_t clutchBitePoint = CLUTCH_DEFAULT_VALUE;
volatile clutchValue_t leftClutchValue = CLUTCH_NONE_VALUE;
volatile clutchValue_t rightClutchValue = CLUTCH_NONE_VALUE;
#define CALIBRATION_INCREMENT 3

// Related to the menu button
#define MENUBTN_PUSH_TICKS 2000 / portTICK_RATE_MS // Two seconds
    static inputBitmap_t menuBtnBitmap = 0;
static TickType_t menuBtnPushTick = 0;
static bool menuRequestInProgress = false;
static bool inConfigMenu = false;

// Related to the autosave feature and user preferences
static esp_timer_handle_t autoSaveTimer = nullptr;
#define PREFS_NAMESPACE "inputHub"
#define KEY_ALT_FUNCTION "af"
#define KEY_CLUTCH_FUNCTION "cf"
#define KEY_CLUTCH_CALIBRATION "ccal"

// Related to POV buttons
// #define DPAD_CENTERED 0
#define DPAD_UP 1
#define DPAD_UP_RIGHT 2
#define DPAD_RIGHT 3
#define DPAD_DOWN_RIGHT 4
#define DPAD_DOWN 5
#define DPAD_DOWN_LEFT 6
#define DPAD_LEFT 7
#define DPAD_UP_LEFT 8
static inputBitmap_t dpadBitmap[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
static inputBitmap_t dpadNegMask = 0;
static inputBitmap_t dpadMask = ~0ULL;

// ----------------------------------------------------------------------------
// Autosave current settings
// ----------------------------------------------------------------------------

void autoSaveCallback(void *param)
{
    Preferences prefs;
    if (prefs.begin(PREFS_NAMESPACE, false))
    {
        prefs.putBool(KEY_ALT_FUNCTION, currentALTFunction);
        prefs.putUChar(KEY_CLUTCH_FUNCTION, (uint8_t)currentClutchFunction);
        prefs.putChar(KEY_CLUTCH_CALIBRATION, (uint8_t)clutchBitePoint);
        prefs.end();
        ui::showSaveNote();
    }
}

void requestSave()
{
    esp_timer_stop(autoSaveTimer);
    esp_timer_start_once(autoSaveTimer, DEFAULT_AUTOSAVE_us);
}

void loadUserSettings()
{
    Preferences prefs;
    if (prefs.begin(PREFS_NAMESPACE, true))
    {
        currentALTFunction = prefs.getBool(KEY_ALT_FUNCTION, currentALTFunction);

        uint8_t value1 = prefs.getUChar(KEY_CLUTCH_FUNCTION, (uint8_t)currentClutchFunction);
        if ((value1 >= CF_CLUTCH) && (value1 <= CF_BUTTON))
        {
            currentClutchFunction = (clutchFunction_t)value1;
        }

        int8_t value2 = prefs.getChar(KEY_CLUTCH_CALIBRATION, (int8_t)clutchBitePoint);
        if ((value2 >= CLUTCH_NONE_VALUE) && (value2 <= CLUTCH_FULL_VALUE))
            clutchBitePoint = (clutchValue_t)value2;

        prefs.end();
    }
}

// ----------------------------------------------------------------------------
// Input Handler
// ----------------------------------------------------------------------------

void inputHub::onStateChanged(inputBitmap_t globalState, inputBitmap_t changes)
{
    // Check if config menu has been requested
    if (changes & menuBtnBitmap)
    {
        TickType_t now = xTaskGetTickCount();
        if ((globalState & menuBtnBitmap) == menuBtnBitmap)
        {
            menuBtnPushTick = now;
            menuRequestInProgress = true;
        }
        else if (menuRequestInProgress && (globalState == 0) && (now > (menuBtnPushTick + MENUBTN_PUSH_TICKS)))
        {
            menuRequestInProgress = false;
            hidImplementation::reset();
            inConfigMenu = configMenu::toggle();
            return;
        }
        else if (globalState == 0)
            menuRequestInProgress = false;
    }
    else
        menuRequestInProgress = false;

    if (inConfigMenu)
    {
        // if config menu is active, route events to another handler
        configMenu::onInput(globalState, changes);
        return;
    }

    inputBitmap_t altFilter;
    bool altEnabled;
    if (currentALTFunction)
    {
        altFilter = altBitmap;
        altEnabled = (globalState & altBitmap);
    }
    else
    {
        altFilter = 0;
        altEnabled = false;
    }

    clutchValue_t clutchValue = CLUTCH_NONE_VALUE;
    if (currentClutchFunction == CF_ALT)
    {
        // Clutch paddles are used as ALT buttons
        altEnabled = altEnabled || ((leftClutchBitmap | rightClutchBitmap) & globalState);
    }
    else if (currentClutchFunction == CF_CLUTCH)
    {
        // F1-Style clutch
        auto leftClutchPaddle = (globalState & leftClutchBitmap);
        auto rightClutchPaddle = (globalState & rightClutchBitmap);

        if (leftClutchPaddle && rightClutchPaddle)
        {
            // Both clutch paddles are pressed
            clutchValue = CLUTCH_FULL_VALUE;
            globalState = globalState & ~(leftClutchBitmap | rightClutchBitmap);
        }
        else if (leftClutchPaddle || rightClutchPaddle)
        {
            // One and only one clutch paddle is pressed
            // Check for fine-grain calibration
            if ((calibrateUpBitmap & changes) &&
                (calibrateUpBitmap & globalState) &&
                (clutchBitePoint < CLUTCH_FULL_VALUE))
            {
                clutchBitePoint += CALIBRATION_INCREMENT;
                requestSave();
            }
            else if ((calibrateDownBitmap & changes) &&
                     (calibrateDownBitmap & globalState) &&
                     (clutchBitePoint > CLUTCH_NONE_VALUE))
            {
                clutchBitePoint -= CALIBRATION_INCREMENT;
                requestSave();
            }
            globalState = globalState & ~(calibrateDownBitmap | calibrateUpBitmap);
            // Set clutch value to bite point
            clutchValue = clutchBitePoint;
            ui::showBitePoint(clutchValue);
        }
    }

    inputBitmap_t filter;
    if (currentClutchFunction == CF_BUTTON)
    {
        // filter = ~(altFilter | menuBtnBitmap);
        filter = ~(altFilter);
    }
    else
    {
        // filter = ~(leftClutchBitmap | rightClutchBitmap | altFilter | menuBtnBitmap);
        filter = ~(leftClutchBitmap | rightClutchBitmap | altFilter);
    }
    globalState = globalState & filter;

    uint8_t povInput = 0;
    if (!altEnabled)
    {
        // Map directional pad buttons to POV input as needed
        inputBitmap_t povState = globalState & dpadNegMask;
        if (povState)
        {
            uint8_t n = 1;
            while ((povInput == 0) && (n < 9))
            {
                if (povState == dpadBitmap[n])
                    povInput = n;
                n++;
            }
        }
        globalState = globalState & (dpadMask);
    }

    hidImplementation::reportInput(globalState, altEnabled, clutchValue, povInput);
}

// ----------------------------------------------------------------------------

void inputHub::onStateChanged(clutchValue_t value, bool leftOrRight, inputNumber_t inputNumber)
{

}

// ----------------------------------------------------------------------------
// Setup
// ----------------------------------------------------------------------------

void inputHub::setClutchPaddles(
    const inputNumber_t leftClutchNumber,
    const inputNumber_t rightClutchNumber)
{
    if ((leftClutchNumber != UNSPECIFIED_INPUT_NUMBER) && (rightClutchNumber != UNSPECIFIED_INPUT_NUMBER))
    {
        leftClutchBitmap = BITMAP(leftClutchNumber);
        rightClutchBitmap = BITMAP(rightClutchNumber);
        capabilities::setFlag(deviceCapability_t::CAP_CLUTCH);
    }
    else
    {
        leftClutchBitmap = 0;
        rightClutchBitmap = 0;
        capabilities::setFlag(deviceCapability_t::CAP_CLUTCH, false);
    }
}

void inputHub::setClutchCalibrationButtons(
    const inputNumber_t upButtonNumber,
    const inputNumber_t downButtonNumber)
{
    if (upButtonNumber != UNSPECIFIED_INPUT_NUMBER)
        calibrateUpBitmap = BITMAP(upButtonNumber);
    else
        calibrateUpBitmap = 0;

    if (downButtonNumber != UNSPECIFIED_INPUT_NUMBER)
        calibrateDownBitmap = BITMAP(downButtonNumber);
    else
        calibrateDownBitmap = 0;
}

void inputHub::setMenuButton(const inputNumber_t menuButtonNumber)
{
    if (menuButtonNumber != UNSPECIFIED_INPUT_NUMBER)
        menuBtnBitmap = BITMAP(menuButtonNumber);
    else
        menuBtnBitmap = 0;
}

void inputHub::setMenuBitmap(const inputBitmap_t menuButtonsBitmap)
{
    menuBtnBitmap = menuButtonsBitmap;
}

void inputHub::setALTBitmap(const inputBitmap_t altBmp)
{
    altBitmap = altBmp;
    capabilities::setFlag(deviceCapability_t::CAP_ALT,(altBitmap!=0);
}

void inputHub::setALTButton(const inputNumber_t altNumber)
{
    if (altNumber == UNSPECIFIED_INPUT_NUMBER)
        altBitmap = 0;
    else
        altBitmap = BITMAP(altNumber);
    capabilities::setFlag(deviceCapability_t::CAP_ALT,(altBitmap!=0);
}

void inputHub::setDPADControls(
    inputNumber_t padUpNumber,
    inputNumber_t padDownNumber,
    inputNumber_t padLeftNumber,
    inputNumber_t padRightNumber,
    inputNumber_t padUpLeftNumber,
    inputNumber_t padUpRightNumber,
    inputNumber_t padDownLeftNumber,
    inputNumber_t padDownRightNumber)
{
    if (padUpNumber < UNSPECIFIED_INPUT_NUMBER)
        dpadBitmap[DPAD_UP] = BITMAP(padUpNumber);
    else
        dpadBitmap[DPAD_UP] = 0;

    if (padDownNumber < UNSPECIFIED_INPUT_NUMBER)
        dpadBitmap[DPAD_DOWN] = BITMAP(padDownNumber);
    else
        dpadBitmap[DPAD_DOWN] = 0;

    if (padLeftNumber < UNSPECIFIED_INPUT_NUMBER)
        dpadBitmap[DPAD_LEFT] = BITMAP(padLeftNumber);
    else
        dpadBitmap[DPAD_LEFT] = 0;

    if (padRightNumber < UNSPECIFIED_INPUT_NUMBER)
        dpadBitmap[DPAD_RIGHT] = BITMAP(padRightNumber);
    else
        dpadBitmap[DPAD_RIGHT] = 0;

    if (padUpLeftNumber < UNSPECIFIED_INPUT_NUMBER)
        dpadBitmap[DPAD_UP_LEFT] = BITMAP(padUpLeftNumber);
    else
        dpadBitmap[DPAD_UP_LEFT] = dpadBitmap[DPAD_UP] | dpadBitmap[DPAD_LEFT];

    if (padUpRightNumber < UNSPECIFIED_INPUT_NUMBER)
        dpadBitmap[DPAD_UP_RIGHT] = BITMAP(padUpRightNumber);
    else
        dpadBitmap[DPAD_UP_RIGHT] = dpadBitmap[DPAD_UP] | dpadBitmap[DPAD_RIGHT];

    if (padDownLeftNumber < UNSPECIFIED_INPUT_NUMBER)
        dpadBitmap[DPAD_DOWN_LEFT] = BITMAP(padDownLeftNumber);
    else
        dpadBitmap[DPAD_DOWN_LEFT] = dpadBitmap[DPAD_DOWN] | dpadBitmap[DPAD_LEFT];

    if (padDownRightNumber < UNSPECIFIED_INPUT_NUMBER)
        dpadBitmap[DPAD_DOWN_RIGHT] = BITMAP(padDownRightNumber);
    else
        dpadBitmap[DPAD_DOWN_RIGHT] = dpadBitmap[DPAD_DOWN] | dpadBitmap[DPAD_RIGHT];

    dpadNegMask = 0;
    for (int n = 1; n < 9; n++)
    {
        dpadNegMask |= dpadBitmap[n];
    }
    dpadMask = ~dpadNegMask;
    capabilities::setFlag(deviceCapability_t::CAP_DPAD,(dpadNegMask!=0);
}

// ----------------------------------------------------------------------------
// Setters
// ----------------------------------------------------------------------------

void inputHub::setClutchBitePoint(clutchValue_t calibrationValue, bool save)
{
    clutchBitePoint = calibrationValue;
    if (save)
        requestSave();
}

void inputHub::setClutchFunction(clutchFunction_t newFunction, bool save)
{
    currentClutchFunction = newFunction;
    if (save)
        requestSave();
}

void inputHub::setALTFunction(bool altFunction, bool save)
{
    currentALTFunction = altFunction;
    if (save)
        requestSave();
}

void inputHub::notifyMenuExit()
{
    inConfigMenu = false;
}

// ----------------------------------------------------------------------------
// Getters
// ----------------------------------------------------------------------------

clutchValue_t inputHub::getClutchBitePoint()
{
    return clutchBitePoint;
}

clutchFunction_t inputHub::getClutchFunction()
{
    return currentClutchFunction;
}

bool inputHub::getALTFunction()
{
    return currentALTFunction;
}

bool inputHub::hasClutchPaddles()
{
    return (leftClutchBitmap > 0) && (rightClutchBitmap > 0);
}

bool inputHub::hasALTButtons()
{
    return (altBitmap != 0);
}

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void inputHub::begin()
{
    if (autoSaveTimer == nullptr)
    {
        esp_timer_create_args_t args;
        args.callback = &autoSaveCallback;
        args.arg = nullptr;
        args.name = nullptr;
        args.dispatch_method = ESP_TIMER_TASK;
        ESP_ERROR_CHECK(esp_timer_create(&args, &autoSaveTimer));
        loadUserSettings();
    }
}