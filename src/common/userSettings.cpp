/**
 * @file userSettings.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-12-21
 * @brief Implementation of the `userSettings` namespace
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.h"
#include "SimWheelTypes.h"
#include <Preferences.h>

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

// Settings (exported for read-only)

volatile clutchValue_t userSettings::bitePoint = CLUTCH_DEFAULT_VALUE;
volatile clutchFunction_t userSettings::cpWorkingMode = clutchFunction_t::CF_CLUTCH;
volatile bool userSettings::altButtonsWorkingMode = true;
volatile bool userSettings::dpadWorkingMode = true;
volatile bool userSettings::securityLock = false;
#define FF_x_4 UNSPECIFIED_INPUT_NUMBER, UNSPECIFIED_INPUT_NUMBER, UNSPECIFIED_INPUT_NUMBER, UNSPECIFIED_INPUT_NUMBER
#define FF_x_16 FF_x_4, FF_x_4, FF_x_4, FF_x_4
#define FF_x_64 FF_x_16, FF_x_16, FF_x_16, FF_x_16
volatile inputNumber_t userSettings::buttonsMap[2][64] = {FF_x_64, FF_x_64};

// Related to the autosave feature and user preferences

static esp_timer_handle_t autoSaveTimer = nullptr;
#define PREFS_NAMESPACE "inputHub" // reused from a previous version
#define KEY_ALT_FUNCTION "af"
#define KEY_CLUTCH_FUNCTION "cf"
#define KEY_CLUTCH_CALIBRATION "ccal"
#define KEY_DPAD_FUNCTION "dpad"
#define KEY_USER_MAP "map"
#define KEY_SECURITY_LOCK "slock"

// ----------------------------------------------------------------------------
// (Auto)save current settings
// ----------------------------------------------------------------------------

void autoSaveCallback(void *param)
{
    Preferences prefs;
    if (prefs.begin(PREFS_NAMESPACE, false))
    {
        prefs.putBool(KEY_ALT_FUNCTION, userSettings::altButtonsWorkingMode);
        prefs.putUChar(KEY_CLUTCH_FUNCTION, (uint8_t)userSettings::cpWorkingMode);
        prefs.putUChar(KEY_CLUTCH_CALIBRATION, (uint8_t)userSettings::bitePoint);
        prefs.putBool(KEY_DPAD_FUNCTION, userSettings::dpadWorkingMode);
        prefs.putBool(KEY_SECURITY_LOCK, userSettings::securityLock);
        prefs.end();
    }
}

void requestSave()
{
    if (autoSaveTimer)
    {
        esp_timer_stop(autoSaveTimer);
        esp_timer_start_once(autoSaveTimer, DEFAULT_AUTOSAVE_us);
    }
}

void userSettings::saveNow()
{
    if (autoSaveTimer)
        esp_timer_stop(autoSaveTimer);
    Preferences prefs;
    if (prefs.begin(PREFS_NAMESPACE, false))
    {
        prefs.putBytes(KEY_USER_MAP, (void *)userSettings::buttonsMap, sizeof(userSettings::buttonsMap));
        prefs.end();
    }
    autoSaveCallback(nullptr);
}

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void userSettings::begin()
{
    if (autoSaveTimer == nullptr)
    {
        esp_timer_create_args_t args;
        args.callback = &autoSaveCallback;
        args.arg = nullptr;
        args.name = nullptr;
        args.dispatch_method = ESP_TIMER_TASK;
        ESP_ERROR_CHECK(esp_timer_create(&args, &autoSaveTimer));

        // Load user settings
        Preferences prefs;
        if (prefs.begin(PREFS_NAMESPACE, true))
        {
            userSettings::securityLock = prefs.getBool(KEY_SECURITY_LOCK, userSettings::securityLock);
            userSettings::altButtonsWorkingMode = prefs.getBool(KEY_ALT_FUNCTION, userSettings::altButtonsWorkingMode);

            uint8_t value1 = prefs.getUChar(KEY_CLUTCH_FUNCTION, (uint8_t)userSettings::cpWorkingMode);
            if ((value1 >= CF_CLUTCH) && (value1 <= CF_BUTTON))
                userSettings::cpWorkingMode = (clutchFunction_t)value1;

            uint8_t value2 = prefs.getUChar(KEY_CLUTCH_CALIBRATION, (uint8_t)userSettings::bitePoint);
            if ((value2 <= CLUTCH_FULL_VALUE)) // && (value2 >= CLUTCH_NONE_VALUE)
                userSettings::bitePoint = (clutchValue_t)value2;

            userSettings::dpadWorkingMode = prefs.getBool(KEY_DPAD_FUNCTION, userSettings::dpadWorkingMode);

            size_t actualSize = prefs.getBytesLength(KEY_USER_MAP);
            if (actualSize == sizeof(userSettings::buttonsMap))
                prefs.getBytes(KEY_USER_MAP, (void *)userSettings::buttonsMap, actualSize);

            prefs.end();
        }
    }
}

// ----------------------------------------------------------------------------
// Setters
// ----------------------------------------------------------------------------

void userSettings::setSecurityLock(bool yesOrNo)
{
    if (userSettings::securityLock != yesOrNo)
    {
        userSettings::securityLock = yesOrNo;
        requestSave();
        hidImplementation::reportChangeInConfig();
        inputs::update();
    }
}

void userSettings::setALTButtonsWorkingMode(bool newMode)
{
    if (userSettings::altButtonsWorkingMode != newMode)
    {
        userSettings::altButtonsWorkingMode = newMode;
        requestSave();
        hidImplementation::reportChangeInConfig();
        inputs::update();
    }
}

void userSettings::setCPWorkingMode(clutchFunction_t newFunction)
{
    if ((newFunction != userSettings::cpWorkingMode) &&
        (newFunction >= CF_CLUTCH) && (newFunction <= CF_BUTTON))
    {
        userSettings::cpWorkingMode = newFunction;
        requestSave();
        hidImplementation::reportChangeInConfig();
        inputs::update();
    }
}

void userSettings::setBitePoint(clutchValue_t newBitePoint)
{
    if (newBitePoint != userSettings::bitePoint)
    {
        if ((newBitePoint != CLUTCH_INVALID_VALUE))
        {
            userSettings::bitePoint = newBitePoint;
            requestSave();
            hidImplementation::reportChangeInConfig();
            notify::bitePoint();
            inputs::update();
        }
    }
}

void userSettings::setDPADWorkingMode(bool newMode)
{
    if (userSettings::dpadWorkingMode != newMode)
    {
        userSettings::dpadWorkingMode = newMode;
        requestSave();
        hidImplementation::reportChangeInConfig();
        inputs::update();
    }
}

void userSettings::setButtonMap(
    inputNumber_t rawInputNumber,
    inputNumber_t userInputNumberNoAlt,
    inputNumber_t userInputNumberAlt)
{
    if (rawInputNumber <= MAX_INPUT_NUMBER)
    {
        if (userInputNumberNoAlt > MAX_USER_INPUT_NUMBER)
            userInputNumberNoAlt = UNSPECIFIED_INPUT_NUMBER;
        if (userInputNumberAlt > MAX_USER_INPUT_NUMBER)
            userInputNumberAlt = UNSPECIFIED_INPUT_NUMBER;
        userSettings::buttonsMap[0][rawInputNumber] = userInputNumberNoAlt;
        userSettings::buttonsMap[1][rawInputNumber] = userInputNumberAlt;
    }
}

void userSettings::resetButtonsMap()
{
    for (int i = 0; i < 64; i++)
    {
        userSettings::buttonsMap[0][i] = UNSPECIFIED_INPUT_NUMBER;
        userSettings::buttonsMap[1][i] = UNSPECIFIED_INPUT_NUMBER;
    }
}

// ----------------------------------------------------------------------------
// Getters
// ----------------------------------------------------------------------------

bool userSettings::getEffectiveButtonMap(inputNumber_t rawInputNumber,
                                         inputNumber_t &userInputNumberNoAlt,
                                         inputNumber_t &userInputNumberAlt)
{
    if (rawInputNumber <= MAX_INPUT_NUMBER)
    {
        userInputNumberNoAlt = buttonsMap[0][rawInputNumber];
        if (userInputNumberNoAlt > MAX_USER_INPUT_NUMBER)
            userInputNumberNoAlt = rawInputNumber;
        userInputNumberAlt = buttonsMap[1][rawInputNumber];
        if (userInputNumberAlt > MAX_USER_INPUT_NUMBER)
            userInputNumberAlt = rawInputNumber + 64;
        return true;
    }
    return false;
}