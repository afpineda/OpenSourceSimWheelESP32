/**
 * @file ServiceMocks.hpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @brief Service mocks for testing
 * @date 2025-03-09
 *
 * @copyright Licensed under the EUPL
 *
 */

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "SimWheelTypes.hpp"
#include "InternalServices.hpp"

/// @cond

//-------------------------------------------------------------------
// Fake classes with user settings
//-------------------------------------------------------------------

class FakeInputService : public InputService
{
public:
    inline static PulseWidthMultiplier _multiplier = PulseWidthMultiplier::_DEFAULT_VALUE;
    inline static int _minLeft = 0;
    inline static int _minRight = 0;
    inline static int _maxLeft = 0;
    inline static int _maxRight = 0;
    inline static bool _leftAxisReversed = false;
    inline static bool _rightAxisReversed = false;

    static void resetToDefaults()
    {
        _multiplier = PulseWidthMultiplier::_DEFAULT_VALUE;
        _minLeft = 0;
        _minRight = 0;
        _maxLeft = 0;
        _maxRight = 0;
        _leftAxisReversed = false;
        _rightAxisReversed = false;
    }

    virtual void setRotaryPulseWidthMultiplier(
        PulseWidthMultiplier multiplier,
        bool save) override
    {
        _multiplier = multiplier;
        if (save)
            SaveSetting::notify(UserSetting::PULSE_WIDTH);
    }

    virtual PulseWidthMultiplier getRotaryPulseWidthMultiplier() override
    {
        return _multiplier;
    }

    virtual bool getAxisCalibration(
        int &minLeft,
        int &maxLeft,
        int &minRight,
        int &maxRight)
    {
        minLeft = _minLeft;
        maxLeft = _maxLeft;
        minRight = _minRight;
        maxRight = _maxRight;
        return true;
    }

    virtual void setAxisCalibration(
        int minLeft,
        int maxLeft,
        int minRight,
        int maxRight,
        bool save)
    {
        _minLeft = minLeft;
        _maxLeft = maxLeft;
        _minRight = minRight;
        _maxRight = maxRight;
        if (save)
            SaveSetting::notify(UserSetting::AXIS_CALIBRATION);
    }

    virtual void getAxisPolarity(
        bool &leftAxisReversed,
        bool &rightAxisReversed) override
    {
        leftAxisReversed = _leftAxisReversed;
        rightAxisReversed = _rightAxisReversed;
    }

    virtual void setAxisPolarity(
        bool leftAxisReversed,
        bool rightAxisReversed,
        bool save) override
    {
        _leftAxisReversed = leftAxisReversed;
        _rightAxisReversed = rightAxisReversed;
        if (save)
            SaveSetting::notify(UserSetting::AXIS_POLARITY);
    }
};

//-------------------------------------------------------------------

class FakeInputHubService : public InputHubService
{
public:
    inline static bool _securityLock = false;
    inline static uint8_t _bitePoint = CLUTCH_NONE_VALUE;
    inline static ClutchWorkingMode _clutchWorkingMode = ClutchWorkingMode::CLUTCH;
    inline static AltButtonsWorkingMode _altButtonsWorkingMode = AltButtonsWorkingMode::ALT;
    inline static DPadWorkingMode _dpadWorkingMode = DPadWorkingMode::Navigation;

    static void resetToDefaults()
    {
        _securityLock = false;
        _bitePoint = CLUTCH_NONE_VALUE;
        _clutchWorkingMode = ClutchWorkingMode::CLUTCH;
        _altButtonsWorkingMode = AltButtonsWorkingMode::ALT;
        _dpadWorkingMode = DPadWorkingMode::Navigation;
    }

    virtual bool getSecurityLock() override
    {
        return _securityLock;
    }

    virtual uint8_t getBitePoint() override
    {
        return _bitePoint;
    }

    virtual ClutchWorkingMode getClutchWorkingMode() override
    {
        return _clutchWorkingMode;
    }

    virtual AltButtonsWorkingMode getAltButtonsWorkingMode() override
    {
        return _altButtonsWorkingMode;
    }

    virtual DPadWorkingMode getDPadWorkingMode() override
    {
        return _dpadWorkingMode;
    }

    virtual void setBitePoint(uint8_t value, bool save) override
    {
        _bitePoint = value;
        OnBitePoint::notify(value);
        if (save)
            SaveSetting::notify(UserSetting::BITE_POINT);
    }

    virtual void setClutchWorkingMode(ClutchWorkingMode mode, bool save) override
    {
        _clutchWorkingMode = mode;
        if (save)
            SaveSetting::notify(UserSetting::CLUTCH_WORKING_MODE);
    }

    virtual void setAltButtonsWorkingMode(AltButtonsWorkingMode mode, bool save) override
    {
        _altButtonsWorkingMode = mode;
        if (save)
            SaveSetting::notify(UserSetting::ALT_WORKING_MODE);
    }

    virtual void setDPadWorkingMode(DPadWorkingMode mode, bool save) override
    {
        _dpadWorkingMode = mode;
        if (save)
            SaveSetting::notify(UserSetting::DPAD_WORKING_MODE);
    }

    virtual void setSecurityLock(bool value, bool save) override
    {
        _securityLock = value;
        if (save)
            SaveSetting::notify(UserSetting::SECURITY_LOCK);
    }
};

//-------------------------------------------------------------------

class FakeHidService : public HidService
{
public:
    inline static uint16_t _customVID = 0;
    inline static uint16_t _customPID = 0;

    static void resetToDefaults()
    {
        _customVID = 0;
        _customPID = 0;
    }

    virtual void getCustomHardwareID(uint16_t &customVID, uint16_t &customPID) override
    {
        customVID = _customVID;
        customPID = _customPID;
    }

    virtual void setCustomHardwareID(uint16_t customVID, uint16_t customPID, bool save) override
    {
        _customVID = customVID;
        _customPID = customPID;
        if (save)
            SaveSetting::notify(UserSetting::CUSTOM_HARDWARE_ID);
    }

    virtual void setFactoryDefaultHardwareID()
    {
        _customVID = 0;
        _customPID = 0;
        SaveSetting::notify(UserSetting::CUSTOM_HARDWARE_ID);
    }
};

//-------------------------------------------------------------------

class FakeBatteryCalibration : public BatteryCalibrationService
{
    inline static int _batteryLevel = UNKNOWN_BATTERY_LEVEL;
    inline static uint16_t _batteryCalibrationQuantum = 103;
    inline static int _maxBatteryReadingEver = 0;

    static void resetToDefaults()
    {
        _batteryLevel = UNKNOWN_BATTERY_LEVEL;
        _batteryCalibrationQuantum = 103;
        _maxBatteryReadingEver = 0;
    }

    virtual int getBatteryLevel(int reading) override
    {
        return _batteryLevel;
    }

    virtual uint8_t getCalibrationDataCount() override
    {
        return 1;
    }

    virtual uint16_t getCalibrationData(uint8_t index) override
    {
        if (index == 0)
            return _batteryCalibrationQuantum;
        else
            return 0;
    }

    virtual void setCalibrationData(uint8_t index, uint16_t data, bool save) override
    {
        if (index == 0)
        {
            _batteryCalibrationQuantum = data;
            if (save)
                SaveSetting::notify(UserSetting::BATTERY_CALIBRATION_DATA);
        }
    }

    virtual int getAutoCalibrationParameter() override
    {
        return _maxBatteryReadingEver;
    }

    virtual void setAutoCalibrationParameter(int value, bool save) override
    {
        _maxBatteryReadingEver = value;
        if (save)
            SaveSetting::notify(UserSetting::BATTERY_AUTO_CALIBRATION);
    }
};

//-------------------------------------------------------------------

class FakeInputMapService : public InputMapService
{
    inline static uint8_t _user_defined = 0xFF;
    inline static uint8_t _user_defined_alt = 0xFF;

    static void resetToDefaults()
    {
        _user_defined = 0xFF;
        _user_defined_alt = 0xFF;
    }

    virtual void setMap(
        uint8_t firmware_defined,
        uint8_t user_defined,
        uint8_t user_defined_alt) override
    {
        _user_defined = user_defined;
        _user_defined_alt = user_defined_alt;
    }

    virtual void getMap(
        uint8_t firmware_defined,
        uint8_t &user_defined,
        uint8_t &user_defined_alt) override
    {
        if (firmware_defined < 64)
        {
            user_defined = _user_defined;
            user_defined_alt = _user_defined_alt;
        }
        else
        {
            user_defined = 0xFF;
            user_defined_alt = 0xFF;
        }
    }
};

/// @endcond
