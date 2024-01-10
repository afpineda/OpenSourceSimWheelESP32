/**
 * @file userSettings.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-12-21
 * @brief Implementation of the `userSettings` namespace
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
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
volatile bool userSettings::altButtonsWorkingMode = false;

// Related to the autosave feature and user preferences

static esp_timer_handle_t autoSaveTimer = nullptr;
#define PREFS_NAMESPACE "inputHub" // reused from a previous version
#define KEY_ALT_FUNCTION "af"
#define KEY_CLUTCH_FUNCTION "cf"
#define KEY_CLUTCH_CALIBRATION "ccal"

// ----------------------------------------------------------------------------
// Autosave current settings
// ----------------------------------------------------------------------------

void autoSaveCallback(void *param)
{
    Preferences prefs;
    if (prefs.begin(PREFS_NAMESPACE, false))
    {
        prefs.putBool(KEY_ALT_FUNCTION, userSettings::altButtonsWorkingMode);
        prefs.putUChar(KEY_CLUTCH_FUNCTION, (uint8_t)userSettings::cpWorkingMode);
        prefs.putUChar(KEY_CLUTCH_CALIBRATION, (uint8_t)userSettings::bitePoint);
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
            userSettings::altButtonsWorkingMode = prefs.getBool(KEY_ALT_FUNCTION, userSettings::altButtonsWorkingMode);

            uint8_t value1 = prefs.getUChar(KEY_CLUTCH_FUNCTION, (uint8_t)userSettings::cpWorkingMode);
            if ((value1 >= CF_CLUTCH) && (value1 <= CF_BUTTON))
                userSettings::cpWorkingMode = (clutchFunction_t)value1;

            uint8_t value2 = prefs.getUChar(KEY_CLUTCH_CALIBRATION, (uint8_t)userSettings::bitePoint);
            if ((value2 >= CLUTCH_NONE_VALUE) && (value2 <= CLUTCH_FULL_VALUE))
                userSettings::bitePoint = (clutchValue_t)value2;

            prefs.end();
        }
    }
}

// ----------------------------------------------------------------------------
// setters
// ----------------------------------------------------------------------------

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
        (newFunction>=CF_CLUTCH) && (newFunction<=CF_BUTTON))
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
            notify::bitePoint(newBitePoint);
            inputs::update();
        }
    }
}
