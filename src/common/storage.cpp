/**
 * @file storage.hpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-20
 * @brief Long-term storage in flash memory
 *
 * @copyright Licensed under the EUPL
 *
 */

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "SimWheelInternals.hpp"
#include "Preferences.h"
#include "InternalServices.hpp"

#if !CD_CI
#include "freertos/FreeRTOS.h" // For esp_timer_handle_t
#endif

#include <atomic>
#include <cstdio>

//-------------------------------------------------------------------
// GLOBALS
//-------------------------------------------------------------------

static const char *SETTINGS_NAMESPACE = "Simwheel";
#define DEFAULT_AUTOSAVE_us 20 * 1000 * 1000
static std::atomic<uint64_t> _pendingSettingsToBeSaved(0ULL);

#if !CD_CI

static esp_timer_handle_t _autoSaveTimer = nullptr;
#define STOP_TIMER esp_timer_stop(_autoSaveTimer)
#define START_TIMER(n) esp_timer_start_once(_autoSaveTimer, n)

#else

#define STOP_TIMER
#define START_TIMER(n) autoSaveCallback(nullptr)

#endif

//-------------------------------------------------------------------

static const char *K_AXIS_CAL_LEFT_MIN = "axisLmin";
static const char *K_AXIS_CAL_LEFT_MAX = "axisLmax";
static const char *K_AXIS_CAL_RIGHT_MIN = "axisRmin";
static const char *K_AXIS_CAL_RIGHT_MAX = "axisRmax";
static const char *K_PULSE_WIDTH = "rotWidth";
static const char *K_AXIS_POLARITY_LEFT = "axisLpol";
static const char *K_AXIS_POLARITY_RIGHT = "axisRpol";
#define DEFAULT_AXIS_CAL_MIN 0
#define DEFAULT_AXIS_CAL_MAX 4095

//-------------------------------------------------------------------

static const char *K_BITE_POINT = "biteP";
static const char *K_CLUTCH_WORKING_MODE = "clutchWM";
static const char *K_ALT_WORKING_MODE = "altWM";
static const char *K_DPAD_WORKING_MODE = "dpadDM";
static const char *K_SECURITY_LOCK = "secLock";

//-------------------------------------------------------------------

static const char *K_CUSTOM_VID = "customVid";
static const char *K_CUSTOM_PID = "customPid";

//-------------------------------------------------------------------

static const char *K_BATT_AUTO_CAL = "battAutoC";
static const char *K_BATT_CALIBRATION_DATA = "battCalD";

//-------------------------------------------------------------------

static const char *K_INPUT_MAP_NO_ALT = "inMapNoAlt_%hhx";
static const char *K_INPUT_MAP_ALT = "inMapAlt_%hhx";

//-------------------------------------------------------------------
// Auxiliary
//-------------------------------------------------------------------

void loadAxisCalibration(Preferences &prefs)
{
    if (prefs.isKey(K_AXIS_CAL_LEFT_MIN) &&
        prefs.isKey(K_AXIS_CAL_LEFT_MAX) &&
        prefs.isKey(K_AXIS_CAL_RIGHT_MIN) &&
        prefs.isKey(K_AXIS_CAL_RIGHT_MAX))
    {
        int minLeft, maxLeft, minRight, maxRight;
        minLeft = prefs.getInt(K_AXIS_CAL_LEFT_MIN, DEFAULT_AXIS_CAL_MIN);
        maxLeft = prefs.getInt(K_AXIS_CAL_LEFT_MAX, DEFAULT_AXIS_CAL_MIN);
        minRight = prefs.getInt(K_AXIS_CAL_RIGHT_MIN, DEFAULT_AXIS_CAL_MAX);
        maxRight = prefs.getInt(K_AXIS_CAL_RIGHT_MAX, DEFAULT_AXIS_CAL_MAX);
        InputService::call::setAxisCalibration(minLeft, maxLeft, minRight, maxRight, false);
    }
}

void saveAxisCalibration(Preferences &prefs)
{
    int minLeft, maxLeft, minRight, maxRight;
    if (!InputService::call::getAxisCalibration(minLeft, maxLeft, minRight, maxRight))
        return;
    prefs.putInt(K_AXIS_CAL_LEFT_MIN, minLeft);
    prefs.putInt(K_AXIS_CAL_LEFT_MAX, maxLeft);
    prefs.putInt(K_AXIS_CAL_RIGHT_MIN, minRight);
    prefs.putInt(K_AXIS_CAL_RIGHT_MAX, maxRight);
}

//-------------------------------------------------------------------

void loadPulseWidth(Preferences &prefs)
{
    if (prefs.isKey(K_PULSE_WIDTH))
    {
        uint8_t value = prefs.getUChar(K_PULSE_WIDTH, (uint8_t)PulseWidthMultiplier::_DEFAULT_VALUE);
        if ((value >= (uint8_t)PulseWidthMultiplier::X1) &&
            (value <= (uint8_t)PulseWidthMultiplier::_MAX_VALUE))
            InputService::call::setRotaryPulseWidthMultiplier((PulseWidthMultiplier)value, false);
    }
}

void savePulseWidth(Preferences &prefs)
{
    uint8_t value = (uint8_t)InputService::call::getRotaryPulseWidthMultiplier();
    prefs.putUChar(K_PULSE_WIDTH, value);
}

//-------------------------------------------------------------------

void loadAxisPolarity(Preferences &prefs)
{
    if (prefs.isKey(K_AXIS_POLARITY_LEFT) && prefs.isKey(K_AXIS_POLARITY_RIGHT))
    {
        bool left = prefs.getBool(K_AXIS_POLARITY_LEFT, false);
        bool right = prefs.getBool(K_AXIS_POLARITY_RIGHT, false);
        InputService::call::setAxisPolarity(left, right, false);
    }
}

void saveAxisPolarity(Preferences &prefs)
{
    bool left, right;
    InputService::call::getAxisPolarity(left, right);
    prefs.putBool(K_AXIS_POLARITY_LEFT, left);
    prefs.putBool(K_AXIS_POLARITY_RIGHT, right);
}

//-------------------------------------------------------------------

void loadSecurityLock(Preferences &prefs)
{
    if (prefs.isKey(K_SECURITY_LOCK))
    {
        bool value = prefs.getBool(K_SECURITY_LOCK, false);
        InputHubService::call::setSecurityLock(value, false);
    }
}

void saveSecurityLock(Preferences &prefs)
{
    prefs.putBool(K_SECURITY_LOCK, InputHubService::call::getSecurityLock());
}

//-------------------------------------------------------------------

void loadBitePoint(Preferences &prefs)
{
    if (prefs.isKey(K_BITE_POINT))
    {
        uint8_t value = prefs.getUChar(K_BITE_POINT, CLUTCH_DEFAULT_VALUE);
        if (value != CLUTCH_INVALID_VALUE)
            InputHubService::call::setBitePoint(value, false);
    }
}

void saveBitePoint(Preferences &prefs)
{
    uint8_t value = InputHubService::call::getBitePoint();
    prefs.putUChar(K_BITE_POINT, value);
}

//-------------------------------------------------------------------

void loadClutchWorkingMode(Preferences &prefs)
{
    if (prefs.isKey(K_CLUTCH_WORKING_MODE))
    {
        uint8_t value = prefs.getUChar(K_CLUTCH_WORKING_MODE, (uint8_t)ClutchWorkingMode::_DEFAULT_VALUE);
        if (value <= (uint8_t)ClutchWorkingMode::_MAX_VALUE)
            InputHubService::call::setClutchWorkingMode(static_cast<ClutchWorkingMode>(value), false);
    }
}

void saveClutchWorkingMode(Preferences &prefs)
{
    prefs.putUChar(K_CLUTCH_WORKING_MODE, (uint8_t)InputHubService::call::getClutchWorkingMode());
}

//-------------------------------------------------------------------

void loadAltWorkingMode(Preferences &prefs)
{
    if (prefs.isKey(K_ALT_WORKING_MODE))
    {
        uint8_t value = prefs.getUChar(K_ALT_WORKING_MODE, (uint8_t)AltButtonsWorkingMode::_DEFAULT_VALUE);
        if (value <= (uint8_t)AltButtonsWorkingMode::_MAX_VALUE)
            InputHubService::call::setAltButtonsWorkingMode(static_cast<AltButtonsWorkingMode>(value), false);
    }
}

void saveAltWorkingMode(Preferences &prefs)
{
    prefs.putUChar(K_ALT_WORKING_MODE, (uint8_t)InputHubService::call::getAltButtonsWorkingMode());
}

//-------------------------------------------------------------------

void loadDpadWorkingMode(Preferences &prefs)
{
    if (prefs.isKey(K_DPAD_WORKING_MODE))
    {
        uint8_t value = prefs.getUChar(K_DPAD_WORKING_MODE, (uint8_t)DPadWorkingMode::_DEFAULT_VALUE);
        if (value <= (uint8_t)DPadWorkingMode::_MAX_VALUE)
            InputHubService::call::setDPadWorkingMode(static_cast<DPadWorkingMode>(value), false);
    }
}

void saveDpadWorkingMode(Preferences &prefs)
{
    prefs.putUChar(K_DPAD_WORKING_MODE, (uint8_t)InputHubService::call::getDPadWorkingMode());
}

//-------------------------------------------------------------------

void loadInputMap(Preferences &prefs)
{
    char key[32];
    uint8_t alt, noAlt;
    for (uint8_t firmware_defined = 0; firmware_defined < 64; firmware_defined++)
    {
        snprintf(key, 32, K_INPUT_MAP_NO_ALT, firmware_defined);
        if (prefs.isKey(key))
        {
            noAlt = prefs.getUChar(key, 0xFF);
            snprintf(key, 32, K_INPUT_MAP_ALT, firmware_defined);
            if (prefs.isKey(key))
            {
                alt = prefs.getUChar(key, 0xFF);
                InputMapService::call::setMap(firmware_defined, noAlt, alt);
            }
        }
    }
}

void saveInputMap(Preferences &prefs)
{
    char key[32];
    uint8_t alt, noAlt;
    for (uint8_t firmware_defined = 0; firmware_defined < 64; firmware_defined++)
    {
        InputMapService::call::getMap(firmware_defined, noAlt, alt);
        if ((noAlt < 128) && (alt < 128))
        {
            snprintf(key, 32, K_INPUT_MAP_NO_ALT, firmware_defined);
            prefs.putUChar(key, noAlt);
            snprintf(key, 32, K_INPUT_MAP_ALT, firmware_defined);
            prefs.putUChar(key, alt);
        }
    }
}

//-------------------------------------------------------------------

void loadCustomHardwareID(Preferences &prefs)
{
    if (prefs.isKey(K_CUSTOM_VID) && prefs.isKey(K_CUSTOM_PID))
    {
        uint16_t vid, pid;
        vid = prefs.getUShort(K_CUSTOM_VID, 0);
        pid = prefs.getUShort(K_CUSTOM_PID, 0);
        HidService::call::setCustomHardwareID(vid, pid, false);
    }
}

void saveCustomHardwareID(Preferences &prefs)
{
    uint16_t vid, pid;
    HidService::call::getCustomHardwareID(vid, pid);
    prefs.putUShort(K_CUSTOM_VID, vid);
    prefs.putUShort(K_CUSTOM_PID, pid);
}

//-------------------------------------------------------------------

void loadBatteryAutoCalibration(Preferences &prefs)
{
    if (prefs.isKey(K_BATT_AUTO_CAL))
    {
        int val = prefs.getUShort(K_BATT_AUTO_CAL, -1);
        BatteryCalibrationService::call::setAutoCalibrationParameter(val, false);
    }
}

void saveBatteryAutoCalibration(Preferences &prefs)
{
    int val = BatteryCalibrationService::call::getAutoCalibrationParameter();
    prefs.putUShort(K_BATT_AUTO_CAL, val);
}

//-------------------------------------------------------------------

void loadBatteryCalibrationData(Preferences &prefs)
{
    if (prefs.isKey(K_BATT_CALIBRATION_DATA))
    {
        size_t storedSize = prefs.getBytesLength(K_BATT_CALIBRATION_DATA);
        uint8_t expectedCount =
            BatteryCalibrationService::call::getCalibrationDataCount();
        if (storedSize == (expectedCount * sizeof(uint16_t)))
        {
            uint16_t loaded[expectedCount];
            prefs.getBytes(K_BATT_CALIBRATION_DATA, loaded, storedSize);
            for (uint8_t i = 0; i < expectedCount; i++)
                BatteryCalibrationService::call::setCalibrationData(i, loaded[i], false);
        }
    }
}

void saveBatteryCalibrationData(Preferences &prefs)
{
    uint8_t expectedCount =
        BatteryCalibrationService::call::getCalibrationDataCount();
    if (expectedCount > 0)
    {
        size_t storedSize = expectedCount * sizeof(uint16_t);
        uint16_t saved[expectedCount];
        for (uint8_t i = 0; i < expectedCount; i++)
            saved[i] = BatteryCalibrationService::call::getCalibrationData(i);
        prefs.putBytes(K_BATT_CALIBRATION_DATA, saved, storedSize);
    }
}

//-------------------------------------------------------------------
// Event Callbacks
//-------------------------------------------------------------------

void autoSaveCallback(void *param)
{
    uint64_t pendingSettingsToBeSaved = _pendingSettingsToBeSaved.exchange(0ULL);
    Preferences prefs;
    if (pendingSettingsToBeSaved && prefs.begin(SETTINGS_NAMESPACE, false))
    {
        for (uint8_t i = (uint8_t)UserSetting::ALL; i <= (uint8_t)UserSetting::_MAX_VALUE; i++)
            if (pendingSettingsToBeSaved & (1ULL << i))
            {
                UserSetting setting = static_cast<UserSetting>(i);
                switch (setting)
                {
                case UserSetting::AXIS_CALIBRATION:
                    saveAxisCalibration(prefs);
                    break;
                case UserSetting::PULSE_WIDTH:
                    savePulseWidth(prefs);
                    break;
                case UserSetting::AXIS_POLARITY:
                    saveAxisPolarity(prefs);
                    break;
                case UserSetting::SECURITY_LOCK:
                    saveSecurityLock(prefs);
                    break;
                case UserSetting::BITE_POINT:
                    saveBitePoint(prefs);
                    break;
                case UserSetting::CLUTCH_WORKING_MODE:
                    saveClutchWorkingMode(prefs);
                    break;
                case UserSetting::ALT_WORKING_MODE:
                    saveAltWorkingMode(prefs);
                    break;
                case UserSetting::DPAD_WORKING_MODE:
                    saveDpadWorkingMode(prefs);
                    break;
                case UserSetting::INPUT_MAP:
                    saveInputMap(prefs);
                    break;
                case UserSetting::CUSTOM_HARDWARE_ID:
                    saveCustomHardwareID(prefs);
                    break;
                case UserSetting::BATTERY_AUTO_CALIBRATION:
                    saveBatteryAutoCalibration(prefs);
                    break;
                case UserSetting::BATTERY_CALIBRATION_DATA:
                    saveBatteryCalibrationData(prefs);
                    break;
                default:
                    break;
                }
            }
        prefs.end();
        OnSettingsSaved::notify();
    }
}

//-------------------------------------------------------------------

void request_save_setting(UserSetting setting)
{
    if (setting == UserSetting::ALL)
        _pendingSettingsToBeSaved.store(~0ULL);
    else
        _pendingSettingsToBeSaved.fetch_or(1ULL << (uint8_t)setting);

    STOP_TIMER;
    switch (setting)
    {
    case UserSetting::ALL:
    case UserSetting::CUSTOM_HARDWARE_ID:
    case UserSetting::BATTERY_AUTO_CALIBRATION:
    case UserSetting::BATTERY_CALIBRATION_DATA:
    case UserSetting::INPUT_MAP:
        // Save now
        START_TIMER(1);
        break;
    default:
        // Save delayed
        START_TIMER(DEFAULT_AUTOSAVE_us);
        break;
    }
}

void load_setting(UserSetting setting)
{
    Preferences prefs;
    if (prefs.begin(SETTINGS_NAMESPACE, true))
    {
        switch (setting)
        {
        case UserSetting::ALL:
            // Used for testing only
            loadAxisCalibration(prefs);
            loadPulseWidth(prefs);
            loadAxisPolarity(prefs);
            loadSecurityLock(prefs);
            loadBitePoint(prefs);
            loadClutchWorkingMode(prefs);
            loadAltWorkingMode(prefs);
            loadDpadWorkingMode(prefs);
            loadInputMap(prefs);
            loadCustomHardwareID(prefs);
            loadBatteryAutoCalibration(prefs);
            loadBatteryCalibrationData(prefs);
            break;
        case UserSetting::AXIS_CALIBRATION:
            loadAxisCalibration(prefs);
            break;
        case UserSetting::PULSE_WIDTH:
            loadPulseWidth(prefs);
            break;
        case UserSetting::AXIS_POLARITY:
            loadAxisPolarity(prefs);
            break;
        case UserSetting::SECURITY_LOCK:
            loadSecurityLock(prefs);
            break;
        case UserSetting::BITE_POINT:
            loadBitePoint(prefs);
            break;
        case UserSetting::CLUTCH_WORKING_MODE:
            loadClutchWorkingMode(prefs);
            break;
        case UserSetting::ALT_WORKING_MODE:
            loadAltWorkingMode(prefs);
            break;
        case UserSetting::DPAD_WORKING_MODE:
            loadDpadWorkingMode(prefs);
            break;
        case UserSetting::INPUT_MAP:
            loadInputMap(prefs);
            break;
        case UserSetting::CUSTOM_HARDWARE_ID:
            loadCustomHardwareID(prefs);
            break;
        case UserSetting::BATTERY_AUTO_CALIBRATION:
            loadBatteryAutoCalibration(prefs);
            break;
        case UserSetting::BATTERY_CALIBRATION_DATA:
            loadBatteryCalibrationData(prefs);
            break;
        default:
            break;
        }
        prefs.end();
    }
}

//-------------------------------------------------------------------
// API
//-------------------------------------------------------------------

void internals::storage::getReady()
{
#if !CD_CI
    if (_autoSaveTimer == nullptr)
#else
    if (!FirmwareService::call::isRunning())
#endif
    {
#if !CD_CI
        esp_timer_create_args_t args;
        args.callback = &autoSaveCallback;
        args.arg = nullptr;
        args.name = nullptr;
        args.dispatch_method = ESP_TIMER_TASK;
        ESP_ERROR_CHECK(esp_timer_create(&args, &_autoSaveTimer));
#endif
        LoadSetting::subscribe(load_setting);
        SaveSetting::subscribe(request_save_setting);
    }
}
