/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-12-21
 * @brief Implementation of the `clutchState` namespace
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

// Clutch state (exported for read-only)

volatile clutchValue_t clutchState::bitePoint = CLUTCH_DEFAULT_VALUE;
volatile clutchFunction_t clutchState::cpWorkingMode = clutchFunction_t::CF_CLUTCH;
volatile bool clutchState::altButtonsWorkingMode = false;

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
        prefs.putBool(KEY_ALT_FUNCTION, clutchState::altButtonsWorkingMode);
        prefs.putUChar(KEY_CLUTCH_FUNCTION, (uint8_t)clutchState::cpWorkingMode);
        prefs.putUChar(KEY_CLUTCH_CALIBRATION, (uint8_t)clutchState::bitePoint);
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

void clutchState::begin()
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
            clutchState::altButtonsWorkingMode = prefs.getBool(KEY_ALT_FUNCTION, clutchState::altButtonsWorkingMode);

            uint8_t value1 = prefs.getUChar(KEY_CLUTCH_FUNCTION, (uint8_t)clutchState::cpWorkingMode);
            if ((value1 >= CF_CLUTCH) && (value1 <= CF_BUTTON))
                clutchState::cpWorkingMode = (clutchFunction_t)value1;

            uint8_t value2 = prefs.getUChar(KEY_CLUTCH_CALIBRATION, (uint8_t)clutchState::bitePoint);
            if ((value2 >= CLUTCH_NONE_VALUE) && (value2 <= CLUTCH_FULL_VALUE))
                clutchState::bitePoint = (clutchValue_t)value2;

            prefs.end();
        }
    }
}

// ----------------------------------------------------------------------------
// setters
// ----------------------------------------------------------------------------

void clutchState::setALTButtonsWorkingMode(bool newMode)
{
    if (clutchState::altButtonsWorkingMode != newMode)
    {
        clutchState::altButtonsWorkingMode = newMode;
        requestSave();
        hidImplementation::reportChangeInConfig();
        inputs::update();
    }
}

void clutchState::setCPWorkingMode(clutchFunction_t newFunction)
{
    if ((newFunction != clutchState::cpWorkingMode) &&
        (newFunction>=CF_CLUTCH) && (newFunction<=CF_BUTTON))
    {
        clutchState::cpWorkingMode = newFunction;
        requestSave();
        hidImplementation::reportChangeInConfig();
        inputs::update();
    }
}

void clutchState::setBitePoint(clutchValue_t newBitePoint)
{
    if (newBitePoint != clutchState::bitePoint)
    {
        if ((newBitePoint != CLUTCH_INVALID_VALUE))
        {
            clutchState::bitePoint = newBitePoint;
            requestSave();
            hidImplementation::reportChangeInConfig();
            notify::bitePoint(newBitePoint);
            inputs::update();
        }
    }
}
