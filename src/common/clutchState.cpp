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

// Effective clutch state (exported for read-only)
volatile clutchValue_t clutchState::leftAxis = CLUTCH_NONE_VALUE;
volatile clutchValue_t clutchState::rightAxis = CLUTCH_NONE_VALUE;
volatile clutchValue_t clutchState::combinedAxis = CLUTCH_NONE_VALUE;
volatile clutchValue_t clutchState::bitePoint = CLUTCH_DEFAULT_VALUE;
volatile clutchFunction_t clutchState::currentFunction = clutchFunction_t::CF_CLUTCH;
volatile bool clutchState::altModeForAltButtons = false;

// Internal clutch state (non exported)
clutchValue_t auxLeftAxis = CLUTCH_NONE_VALUE;
clutchValue_t auxRightAxis = CLUTCH_NONE_VALUE;

// Related to the autosave feature and user preferences
static esp_timer_handle_t autoSaveTimer = nullptr;
#define PREFS_NAMESPACE "inputHub" // reused from a previous version
#define KEY_ALT_FUNCTION "af"
#define KEY_CLUTCH_FUNCTION "cf"
#define KEY_CLUTCH_CALIBRATION "ccal"

// ----------------------------------------------------------------------------
// Auxiliary
// ----------------------------------------------------------------------------

void inline updateCombinedAxis()
{
    if (auxLeftAxis > auxRightAxis)
        clutchState::combinedAxis =
            (auxLeftAxis * clutchState::bitePoint +
             (auxRightAxis * (255 - clutchState::bitePoint))) /
            255;
    else
        clutchState::combinedAxis =
            (auxRightAxis * clutchState::bitePoint +
             (auxLeftAxis * (255 - clutchState::bitePoint))) /
            255;
}

void inline update()
{
    switch (clutchState::currentFunction)
    {
    case CF_CLUTCH:
        clutchState::leftAxis = CLUTCH_NONE_VALUE;
        clutchState::rightAxis = CLUTCH_NONE_VALUE;
        updateCombinedAxis();
        break;
    case CF_AXIS:
        clutchState::leftAxis = auxLeftAxis;
        clutchState::rightAxis = auxRightAxis;
        clutchState::combinedAxis = CLUTCH_NONE_VALUE;
        break;
    case CF_ALT:
    case CF_BUTTON:
        clutchState::leftAxis = CLUTCH_NONE_VALUE;
        clutchState::rightAxis = CLUTCH_NONE_VALUE;
        clutchState::combinedAxis = CLUTCH_NONE_VALUE;
        break;
    default:
        break;
    }
}

// ----------------------------------------------------------------------------
// Autosave current settings
// ----------------------------------------------------------------------------

void autoSaveCallback(void *param)
{
    Preferences prefs;
    if (prefs.begin(PREFS_NAMESPACE, false))
    {
        prefs.putBool(KEY_ALT_FUNCTION, clutchState::altModeForAltButtons);
        prefs.putUChar(KEY_CLUTCH_FUNCTION, (uint8_t)clutchState::currentFunction);
        prefs.putUChar(KEY_CLUTCH_CALIBRATION, (uint8_t)clutchState::bitePoint);
        prefs.end();
        // ui::showSaveNote();
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
            clutchState::altModeForAltButtons = prefs.getBool(KEY_ALT_FUNCTION, clutchState::altModeForAltButtons);

            uint8_t value1 = prefs.getUChar(KEY_CLUTCH_FUNCTION, (uint8_t)clutchState::currentFunction);
            if ((value1 >= CF_CLUTCH) && (value1 <= CF_BUTTON))
                clutchState::currentFunction = (clutchFunction_t)value1;

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

void clutchState::setALTModeForButtons(bool newMode)
{
    if (clutchState::altModeForAltButtons != newMode)
    {
        clutchState::altModeForAltButtons = newMode;
        requestSave();
        hidImplementation::reportChangeInConfig();
    }
}

void clutchState::setFunction(clutchFunction_t newFunction)
{
    if (newFunction != clutchState::currentFunction)
    {
        clutchState::currentFunction = newFunction;
        update();
        requestSave();
        hidImplementation::reportChangeInConfig();
    }
}

void clutchState::setBitePoint(clutchValue_t newBitePoint)
{
    if (newBitePoint != clutchState::bitePoint)
    {
        clutchState::bitePoint = newBitePoint;
        update();
        requestSave();
        hidImplementation::reportChangeInConfig();
    }
}

void clutchState::setLeftAxis(clutchValue_t newValue)
{
    auxLeftAxis = newValue;
    update();
}

void clutchState::setRightAxis(clutchValue_t newValue)
{
    auxRightAxis = newValue;
    update();
}

// ----------------------------------------------------------------------------
// checks
// ----------------------------------------------------------------------------

bool clutchState::isALTRequested()
{
    return (clutchState::currentFunction == CF_ALT) &&
           ((auxLeftAxis >= CLUTCH_DEFAULT_VALUE) || (auxRightAxis >= CLUTCH_DEFAULT_VALUE));
}

bool clutchState::isCalibrationInProgress()
{
    return (clutchState::currentFunction == CF_CLUTCH) &&
           ((auxLeftAxis >= CLUTCH_DEFAULT_VALUE) ^
            (auxRightAxis >= CLUTCH_DEFAULT_VALUE));
}