/**
 * @file Report3Test.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-10
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
#include "HID_definitions.hpp"
#include <cinttypes>
#include <cassert>

//-------------------------------------------------------------------
// Mocks
//-------------------------------------------------------------------

class InputHubServiceMock : public InputHubService
{
public:
    inline static bool securityLock = false;
    inline static uint8_t bitePoint = CLUTCH_NONE_VALUE;
    inline static ClutchWorkingMode clutchWorkingMode = ClutchWorkingMode::CLUTCH;
    inline static AltButtonsWorkingMode altButtonsWorkingMode = AltButtonsWorkingMode::ALT;
    inline static DPadWorkingMode dpadWorkingMode = DPadWorkingMode::Navigation;

    virtual bool getSecurityLock() override
    {
        return securityLock;
    }

    virtual uint8_t getBitePoint() override
    {
        return bitePoint;
    }

    virtual ClutchWorkingMode getClutchWorkingMode() override
    {
        return clutchWorkingMode;
    }

    virtual AltButtonsWorkingMode getAltButtonsWorkingMode() override
    {
        return altButtonsWorkingMode;
    }

    virtual DPadWorkingMode getDPadWorkingMode() override
    {
        return dpadWorkingMode;
    }

    virtual void setBitePoint(uint8_t value, bool save) override
    {
        if ((value < CLUTCH_INVALID_VALUE) && (value != bitePoint))
        {
            bitePoint = value;
        }
    }

    virtual void setClutchWorkingMode(ClutchWorkingMode mode, bool save) override
    {
        if (mode != clutchWorkingMode)
        {
            clutchWorkingMode = mode;
        }
    }

    virtual void setAltButtonsWorkingMode(AltButtonsWorkingMode mode, bool save) override
    {
        if (mode != altButtonsWorkingMode)
        {
            altButtonsWorkingMode = mode;
        }
    }

    virtual void setDPadWorkingMode(DPadWorkingMode mode, bool save) override
    {
        if (mode != dpadWorkingMode)
        {
            dpadWorkingMode = mode;
        }
    }

    virtual void setSecurityLock(bool value, bool save) override
    {
        if (securityLock != value)
        {
            securityLock = value;
        }
    }
} inputHubMock;

class InputServiceMock : public InputService
{
public:
    inline static PulseWidthMultiplier _fakePulseWidth = PulseWidthMultiplier::_DEFAULT_VALUE;
    inline static bool _reverseLeftAxis = false;
    inline static bool _reverseRightAxis = false;

    virtual void recalibrateAxes() override
    {
    }

    virtual void reverseLeftAxis() override
    {
        _reverseLeftAxis = !_reverseLeftAxis;
    }

    virtual void reverseRightAxis() override
    {
        _reverseRightAxis = !_reverseRightAxis;
    }

    virtual void setRotaryPulseWidthMultiplier(
        PulseWidthMultiplier multiplier,
        bool save) override
    {
        _fakePulseWidth = multiplier;
    }

    virtual PulseWidthMultiplier getRotaryPulseWidthMultiplier() override
    {
        return _fakePulseWidth;
    };

    virtual bool getAxisCalibration(
        int &minLeft,
        int &maxLeft,
        int &minRight,
        int &maxRight)
    {
        return false;
    }

    virtual void setAxisCalibration(
        int minLeft,
        int maxLeft,
        int minRight,
        int maxRight,
        bool save)
    {
    }

    virtual void getAxisPolarity(
        bool &leftAxisReversed,
        bool &rightAxisReversed) override
    {
    }

    virtual void setAxisPolarity(
        bool leftAxisReversed,
        bool rightAxisReversed,
        bool save) override
    {
        _reverseLeftAxis = leftAxisReversed;
        _reverseRightAxis = rightAxisReversed;
    }

    virtual void update() {};
} inputMock;

//-------------------------------------------------------------------
// Auxiliary
//-------------------------------------------------------------------

typedef struct __attribute__((packed))
{
    uint8_t cp_wm = 0xFF;
    uint8_t alt_wm = 0xFF;
    uint8_t bitePoint = 0xFF;
    uint8_t simpleCmd = 0xFF;
    uint8_t dpad_wm = 0xFF;
    uint8_t secLock = 0xFF;
    uint8_t pulseWidth = 0xFF;
} Report3;

#define REPORT3BYTES(s) ((uint8_t *)&s)

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Test groups
//-------------------------------------------------------------------
//-------------------------------------------------------------------

void TG_read()
{
    Report3 rep3;
    assert((rep3.pulseWidth == 0xFF) && "Report 3 not initialized");
    internals::hid::common::onGetFeature(
        RID_FEATURE_CONFIG, REPORT3BYTES(rep3), sizeof(rep3));
    assert((rep3.cp_wm == (uint8_t)inputHubMock.clutchWorkingMode) && "Clutch WM not set");
    assert((rep3.alt_wm == (uint8_t)inputHubMock.altButtonsWorkingMode) && "ALT WM not set");
    assert((rep3.bitePoint == (uint8_t)inputHubMock.bitePoint) && "Bite point not set");
    assert((rep3.simpleCmd == (uint8_t)UNKNOWN_BATTERY_LEVEL) && "Soc not set");
    assert((rep3.dpad_wm == (uint8_t)inputHubMock.dpadWorkingMode) && "DPAD WM not set");
    assert((rep3.secLock == (uint8_t)inputHubMock.securityLock) && "Security lock not set");
    assert((rep3.pulseWidth == (uint8_t)inputMock._fakePulseWidth) && "Pulse width not set");
}

void TG_SimpleCommand()
{
    Report3 rep3;
    bool aux = !inputMock._reverseLeftAxis;
    rep3.simpleCmd = 5;
    internals::hid::common::onSetFeature(
        RID_FEATURE_CONFIG, REPORT3BYTES(rep3), sizeof(rep3));
    assert((inputMock._reverseLeftAxis == aux) && "CMD failure: reverse left axis");

    aux = !inputMock._reverseRightAxis;
    rep3.simpleCmd = 6;
    internals::hid::common::onSetFeature(
        RID_FEATURE_CONFIG, REPORT3BYTES(rep3), sizeof(rep3));
    assert((inputMock._reverseRightAxis == aux) && "CMD failure: reverse right axis");
}

void TG_write()
{
    Report3 rep3a, rep3b;
    inputHubMock.securityLock = false;
    inputHubMock.bitePoint = CLUTCH_NONE_VALUE;
    inputHubMock.clutchWorkingMode = ClutchWorkingMode::CLUTCH;
    inputHubMock.altButtonsWorkingMode = AltButtonsWorkingMode::ALT;
    inputHubMock.dpadWorkingMode = DPadWorkingMode::Navigation;

    rep3a.cp_wm = (uint8_t)ClutchWorkingMode::BUTTON;
    rep3a.alt_wm = (uint8_t)AltButtonsWorkingMode::Regular;
    rep3a.bitePoint = CLUTCH_3_4_VALUE;
    internals::hid::common::onSetFeature(
        RID_FEATURE_CONFIG, REPORT3BYTES(rep3a), sizeof(rep3a));

    assert((rep3a.cp_wm == (uint8_t)inputHubMock.clutchWorkingMode) && "Write failure: clutch WM");
    assert((rep3a.alt_wm == (uint8_t)inputHubMock.altButtonsWorkingMode) && "Write failure: alt WM");
    assert((rep3a.bitePoint == (uint8_t)inputHubMock.bitePoint) && "Write failure: bitepoint");
    assert((inputHubMock.dpadWorkingMode == DPadWorkingMode::Navigation) && "Unexpected Write: dpad WM");

    rep3b.dpad_wm = (uint8_t)DPadWorkingMode::Regular;
    rep3b.pulseWidth = 3;
    rep3b.secLock = true;
    internals::hid::common::onSetFeature(
        RID_FEATURE_CONFIG, REPORT3BYTES(rep3b), sizeof(rep3b));

    assert((inputHubMock.clutchWorkingMode == ClutchWorkingMode::BUTTON) && "Unexpected Write: clutch WM");
    assert((inputHubMock.altButtonsWorkingMode == AltButtonsWorkingMode::Regular) && "Unexpected Write: alt WM");
    assert((inputHubMock.bitePoint == CLUTCH_3_4_VALUE) && "Unexpected Write: bitepoint");
    assert((rep3b.dpad_wm == (uint8_t)inputHubMock.dpadWorkingMode) && "Write failure: dpad WM");
    assert((rep3b.pulseWidth == (uint8_t)inputMock._fakePulseWidth) && "Write failure: pulse width");
    assert((!inputHubMock.securityLock) && "Security lock is not read-only");
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------
//-------------------------------------------------------------------

int main()
{
    assert((sizeof(Report3) == CONFIG_REPORT_SIZE) && "Test is outdated");
    InputHubService::inject(&inputHubMock);
    InputService::inject(&inputMock);

    TG_read();
    TG_SimpleCommand();
    TG_write();

    return 0;
}