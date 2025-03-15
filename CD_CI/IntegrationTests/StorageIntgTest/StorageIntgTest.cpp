/**
 * @file StorageIntgTest.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-03-01
 * @brief Integration test
 *
 * @copyright Licensed under the EUPL
 *
 */

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"

#include <iostream>

//-------------------------------------------------------------------
// Mocks
//-------------------------------------------------------------------

class InputServiceMock : public InputService
{
public:
    virtual void setRotaryPulseWidthMultiplier(
        PulseWidthMultiplier multiplier,
        bool save) override { _pulseLoaded = !save; }

    virtual PulseWidthMultiplier getRotaryPulseWidthMultiplier() override
    {
        _pulseSaved = true;
        return this->InputService::getRotaryPulseWidthMultiplier();
    }

    virtual bool getAxisCalibration(
        int &minLeft,
        int &maxLeft,
        int &minRight,
        int &maxRight)
    {
        _axisCalSaved = true;
        minLeft = 0;
        maxLeft = 1000;
        minRight = 0;
        maxRight = 1000;
        return true;
    }

    virtual void setAxisCalibration(
        int minLeft,
        int maxLeft,
        int minRight,
        int maxRight,
        bool save) override { _axisCalLoaded = !save; }

    virtual void getAxisPolarity(
        bool &leftAxisReversed,
        bool &rightAxisReversed) override
    {
        _polaritySaved = true;
        leftAxisReversed = true;
        rightAxisReversed = true;
    }

    virtual void setAxisPolarity(
        bool leftAxisReversed,
        bool rightAxisReversed,
        bool save) override { _polarityLoaded = !save; }

    inline static bool _pulseSaved = false;
    inline static bool _pulseLoaded = false;
    inline static bool _axisCalSaved = false;
    inline static bool _axisCalLoaded = false;
    inline static bool _polarityLoaded = false;
    inline static bool _polaritySaved = false;
} inputMock;

//-------------------------------------------------------------------

class InputHubMock : public InputHubService
{
public:
    inline static bool _secLockSaved = false;
    inline static bool _secLockLoaded = false;
    inline static bool _bpSaved = false;
    inline static bool _bpLoaded = false;
    inline static bool _clutchWMSaved = false;
    inline static bool _clutchWMLoaded = false;
    inline static bool _altWMSaved = false;
    inline static bool _altWMLoaded = false;
    inline static bool _dpadWMSaved = false;
    inline static bool _dpadWMLoaded = false;

    virtual bool getSecurityLock() override
    {
        _secLockSaved = true;
        return false;
    }
    virtual void setSecurityLock(bool value, bool save) { _secLockLoaded = !save; }

    virtual uint8_t getBitePoint() override
    {
        _bpSaved = true;
        return 0;
    }
    virtual void setBitePoint(uint8_t value, bool save) override { _bpLoaded = !save; }

    virtual ClutchWorkingMode getClutchWorkingMode()
    {
        _clutchWMSaved = true;
        return ClutchWorkingMode::_DEFAULT_VALUE;
    }
    virtual void setClutchWorkingMode(ClutchWorkingMode mode, bool save) override { _clutchWMLoaded = !save; }

    virtual AltButtonsWorkingMode getAltButtonsWorkingMode()
    {
        _altWMSaved = true;
        return AltButtonsWorkingMode::_DEFAULT_VALUE;
    }
    virtual void setAltButtonsWorkingMode(AltButtonsWorkingMode mode, bool save) override { _altWMLoaded = !save; }

    virtual DPadWorkingMode getDPadWorkingMode()
    {
        _dpadWMSaved = true;
        return DPadWorkingMode::_DEFAULT_VALUE;
    }
    virtual void setDPadWorkingMode(DPadWorkingMode mode, bool save) override { _dpadWMLoaded = !save; }
} inputHubMock;

//-------------------------------------------------------------------

class InputMapMock : public InputMapService
{
public:
    inline static bool _saved = false;
    inline static bool _loaded = false;

    virtual void setMap(
        uint8_t firmware_defined,
        uint8_t user_defined,
        uint8_t user_defined_alt) { _loaded = true; }
    virtual void getMap(
        uint8_t firmware_defined,
        uint8_t &user_defined,
        uint8_t &user_defined_alt)
    {
        _saved = true;
        user_defined = 0;
        user_defined_alt = 0;
    }
    virtual void resetMap() MOCK;
} inputMapMock;

//-------------------------------------------------------------------

class BattCalMock : public BatteryCalibrationService
{
public:
    inline static bool _calDataSaved = false;
    inline static bool _calDataLoaded = false;
    inline static bool _autoParamSaved = false;
    inline static bool _autoParamLoaded = false;

    virtual uint8_t getCalibrationDataCount() MOCK_R(1);

    virtual uint16_t getCalibrationData(uint8_t index)
    {
        _calDataSaved = true;
        return 16;
    }

    virtual void setCalibrationData(uint8_t index, uint16_t data, bool save)
    {
        _calDataLoaded = !save;
    }

    virtual int getAutoCalibrationParameter()
    {
        _autoParamSaved = true;
        return 100;
    }

    virtual void setAutoCalibrationParameter(int value, bool save)
    {
        _autoParamLoaded = !save;
    }
} battCalMock;

//-------------------------------------------------------------------

class HidMock : public HidService
{
public:
    inline static bool _saved = false;
    inline static bool _loaded = false;

    virtual void getCustomHardwareID(
        uint16_t &customVID,
        uint16_t &customPID) override
    {
        _saved = true;
        customVID = 100;
        customPID = 100;
    }

    virtual void setCustomHardwareID(
        uint16_t customVID,
        uint16_t customPID,
        bool save) override { _loaded = !save; }
} hidMock;

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Test groups
//-------------------------------------------------------------------
//-------------------------------------------------------------------

void TG_Inputs()
{
    // std::cout << "- Pulse width -" << std::endl;
    SaveSetting::notify(UserSetting::PULSE_WIDTH);
    assert((InputServiceMock::_pulseSaved) && "Pulse width not saved");
    LoadSetting::notify(UserSetting::PULSE_WIDTH);
    assert((InputServiceMock::_pulseLoaded) && "Pulse width not loaded");

    // std::cout << "- Axis calibration -" << std::endl;
    SaveSetting::notify(UserSetting::AXIS_CALIBRATION);
    assert((InputServiceMock::_axisCalSaved) && "Axis calibration not saved");
    LoadSetting::notify(UserSetting::AXIS_CALIBRATION);
    assert((InputServiceMock::_axisCalLoaded) && "Axis calibration not loaded");

    SaveSetting::notify(UserSetting::AXIS_POLARITY);
    assert((InputServiceMock::_polaritySaved) && "Axis polarity not saved");
    LoadSetting::notify(UserSetting::AXIS_POLARITY);
    assert((InputServiceMock::_polarityLoaded) && "Axis polarity not loaded");
}

//-------------------------------------------------------------------

void TG_InputHub()
{
    SaveSetting::notify(UserSetting::SECURITY_LOCK);
    assert((InputHubMock::_secLockSaved) && "Sec lock not saved");
    LoadSetting::notify(UserSetting::SECURITY_LOCK);
    assert((InputHubMock::_secLockLoaded) && "Sec lock not loaded");

    SaveSetting::notify(UserSetting::BITE_POINT);
    assert((InputHubMock::_bpSaved) && "Bite point not saved");
    LoadSetting::notify(UserSetting::BITE_POINT);
    assert((InputHubMock::_bpLoaded) && "Bite point not loaded");

    SaveSetting::notify(UserSetting::CLUTCH_WORKING_MODE);
    assert((InputHubMock::_clutchWMSaved) && "clutch wm not saved");
    LoadSetting::notify(UserSetting::CLUTCH_WORKING_MODE);
    assert((InputHubMock::_clutchWMLoaded) && "clutch wm not loaded");

    SaveSetting::notify(UserSetting::ALT_WORKING_MODE);
    assert((InputHubMock::_altWMSaved) && "Alt wm not saved");
    LoadSetting::notify(UserSetting::ALT_WORKING_MODE);
    assert((InputHubMock::_altWMLoaded) && "Alt wm not loaded");

    SaveSetting::notify(UserSetting::DPAD_WORKING_MODE);
    assert((InputHubMock::_dpadWMSaved) && "dpad wm not saved");
    LoadSetting::notify(UserSetting::DPAD_WORKING_MODE);
    assert((InputHubMock::_dpadWMLoaded) && "dpad wm not loaded");
}

//-------------------------------------------------------------------

void TG_InputMap()
{
    SaveSetting::notify(UserSetting::INPUT_MAP);
    assert((InputMapMock::_saved) && "Map not saved");
    LoadSetting::notify(UserSetting::INPUT_MAP);
    assert((InputMapMock::_loaded) && "Map not loaded");
}

//-------------------------------------------------------------------

void TG_BattCal()
{
    SaveSetting::notify(UserSetting::BATTERY_AUTO_CALIBRATION);
    assert((BattCalMock::_autoParamSaved) && "Batt autocal not saved");
    LoadSetting::notify(UserSetting::BATTERY_AUTO_CALIBRATION);
    assert((BattCalMock::_autoParamLoaded) && "Batt autocal not loaded");

    SaveSetting::notify(UserSetting::BATTERY_CALIBRATION_DATA);
    assert((BattCalMock::_calDataSaved) && "Batt data not saved");
    LoadSetting::notify(UserSetting::BATTERY_CALIBRATION_DATA);
    assert((BattCalMock::_calDataLoaded) && "Batt data loaded");
}

//-------------------------------------------------------------------

void TG_Hid()
{
    SaveSetting::notify(UserSetting::CUSTOM_HARDWARE_ID);
    assert((HidMock::_saved) && "Hardware id not saved");
    LoadSetting::notify(UserSetting::CUSTOM_HARDWARE_ID);
    assert((HidMock::_loaded) && "Hardware id not loaded");
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------
//-------------------------------------------------------------------

int main()
{
    InputService::inject(&inputMock);
    InputHubService::inject(&inputHubMock);
    InputMapService::inject(&inputMapMock);
    BatteryCalibrationService::inject(&battCalMock);
    HidService::inject(&hidMock);

    internals::storage::getReady();
    OnStart::notify();

    TG_Inputs();
    TG_InputHub();
    TG_InputMap();
    TG_BattCal();
    TG_Hid();

    return 0;
}
