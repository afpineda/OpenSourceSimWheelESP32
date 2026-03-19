/**
 * @file InputHubIntgTest.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-03-06
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
#include "cd_ci_assertions.hpp"
#include <cinttypes>
#include <cassert>
#include <iostream>

//-------------------------------------------------------------------
// Globals
//-------------------------------------------------------------------

#define L_CLUTCH 0
#define R_CLUTCH 1
#define RECALIBRATE1 2
#define RECALIBRATE2 3
#define BITE_POINT_UP 4
#define BITE_POINT_DOWN 5
#define ALT 6
#define CYCLE 7
#define BUTTON1 30

#define BMP(n) (1ULL << n)
#define BMP2(n, m) BMP(n) | BMP(m)

//-------------------------------------------------------------------
// Mocks
//-------------------------------------------------------------------

// Force battery auto-calibration with a button combination
// was removed

// class BattCalMock : public BatteryCalibrationService
// {
// public:
//     bool autocalibrationWitness = false;
//     virtual void restartAutoCalibration() override
//     {
//         autocalibrationWitness = true;
//     }
// } battCalMock;

//-------------------------------------------------------------------

class InputMock : public InputService
{
public:
    bool recalibrateWitness = false;
    virtual void recalibrateAxes() override
    {
        recalibrateWitness = true;
    }
} inputMock;

//-------------------------------------------------------------------

uint8_t bitePointWitness = 0;

void bitePointCallback(uint8_t value)
{
    bitePointWitness = value;
}

//-------------------------------------------------------------------

void internals::inputMap::clear() {}

void internals::inputMap::getReady() {}

size_t mapWitnessCount = 0;
uint128_t mapBitmapWitness{};

void internals::inputMap::map(
    bool isAltModeEngaged,
    uint128_t &bitmap)
{
    mapWitnessCount++;
    mapBitmapWitness = bitmap;
    if (isAltModeEngaged)
    {
        bitmap.high = bitmap.low;
        bitmap.low = 0ULL;
    }
    else
    {
        bitmap.high = 0ULL;
    }
}

void inputMap::set(
    InputNumber firmware_defined,
    InputNumber user_defined,
    InputNumber user_define_alt_engaged) {}

//-------------------------------------------------------------------

size_t reportWitness = 0;

bool internals::hid::isConnected() { return true; }
bool internals::hid::supportsCustomHardwareID() { return true; }
void internals::hid::reportChangeInConfig() {}
void internals::hid::reportBatteryLevel(const BatteryStatus &status) {}
void internals::hid::reportInput(
    const uint128_t &input,
    uint8_t POVstate,
    uint8_t leftAxis,
    uint8_t rightAxis,
    uint8_t clutchAxis)
{
    reportWitness++;
}

void internals::hid::reset() {}

void internals::hid::begin(
    std::string deviceName,
    std::string deviceManufacturer,
    bool enableAutoPowerOff,
    uint16_t vendorID,
    uint16_t productID,
    bool usb_enable,
    bool ble_enable,
    bool exclusive) {}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Test groups
//-------------------------------------------------------------------
//-------------------------------------------------------------------

/**
 * @brief Test axis recalibration using a button combination
 *
 */
void TG_axis_recalibration()
{
    DecouplingEvent evt{};
    evt.rawInputBitmap.low = BMP(RECALIBRATE1);
    internals::inputHub::onRawInput(evt);
    assert<bool>::equals("Unwanted axis recalibration 1", false, inputMock.recalibrateWitness);

    evt.leftAxisValue = 0;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap.low = BMP(RECALIBRATE2);
    internals::inputHub::onRawInput(evt);
    assert<bool>::equals("Unwanted axis recalibration 2", false, inputMock.recalibrateWitness);

    evt.leftAxisValue = 0;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap.low = BMP2(RECALIBRATE1, RECALIBRATE2);
    internals::inputHub::onRawInput(evt);
    assert<bool>::equals("Recalibration request", true, inputMock.recalibrateWitness);

    evt.leftAxisValue = 0;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap.low = BMP2(RECALIBRATE1, RECALIBRATE2);
    inputMock.recalibrateWitness = false;
    internals::inputHub::onRawInput(evt);
    assert<bool>::equals("Recalibration request retrigger", false, inputMock.recalibrateWitness);

    evt.leftAxisValue = 0;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap.low = 0b0000;
    internals::inputHub::onRawInput(evt);
    assert<bool>::equals("Unwanted axis recalibration 3", false, inputMock.recalibrateWitness);
}

/**
 * @brief Test the bite point event is triggered
 *
 */
void TG_bite_point()
{
    DecouplingEvent evt{};
    bitePointWitness = 0;
    InputHubService::call::setBitePoint(CLUTCH_DEFAULT_VALUE);
    assert<int>::equals("Bite point initial state and callback", CLUTCH_DEFAULT_VALUE, bitePointWitness);

    evt.leftAxisValue = 254;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap.low = BMP(BITE_POINT_UP); // Btn 4 press
    internals::inputHub::onRawInput(evt);
    evt.rawInputBitmap.low = 0ULL;
    internals::inputHub::onRawInput(evt);
    assert<int>::more("Bite point up callback", CLUTCH_DEFAULT_VALUE, bitePointWitness);

    InputHubService::call::setBitePoint(CLUTCH_DEFAULT_VALUE);
    evt.leftAxisValue = 254;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap.low = BMP(BITE_POINT_DOWN); // Btn 5 press
    internals::inputHub::onRawInput(evt);
    evt.rawInputBitmap.low = 0ULL;
    internals::inputHub::onRawInput(evt);
    assert<int>::less("Bite point down callback", CLUTCH_DEFAULT_VALUE, bitePointWitness);
}

/**
 * @brief Check that inputMap is called
 *
 */
void TG_map()
{
    DecouplingEvent evt{};
    mapWitnessCount = 0;
    internals::inputHub::onRawInput(evt);
    assert<size_t>::equals("map() call 1", 1, mapWitnessCount);

    evt.leftAxisValue = 0;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap.low = BMP(BUTTON1);
    internals::inputHub::onRawInput(evt);
    assert<size_t>::equals("map() call 2", 2, mapWitnessCount);
    assert<uint64_t>::equals(
        "map() call 2 parameter",
        BMP(BUTTON1),
        mapBitmapWitness.low);

    evt.leftAxisValue = 0;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap.low = BMP(ALT);
    internals::inputHub::onRawInput(evt);
    assert<size_t>::equals("map() call 3", 3, mapWitnessCount);
    assert<uint64_t>::equals(
        "map() call 3 parameter",
        0ULL,
        mapBitmapWitness.low);

    evt.leftAxisValue = 0;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap.low = BMP(CYCLE);
    internals::inputHub::onRawInput(evt);
    assert<size_t>::equals(
        "unexpected map() call",
        3,
        mapWitnessCount); // not called
}

/**
 * @brief Check that hid is called
 *
 */
void TG_hid()
{
    reportWitness = 0;
    {
        DecouplingEvent evt{};
        internals::inputHub::onRawInput(evt);
        assert<size_t>::equals("reportInput() call 1", 1, reportWitness);
    }
    {
        DecouplingEvent evt{};
        evt.leftAxisValue = 100;
        internals::inputHub::onRawInput(evt);
        assert<size_t>::equals("reportInput() call 2", 2, reportWitness);
    }
    {
        DecouplingEvent evt{};
        evt.rightAxisValue = 99;
        internals::inputHub::onRawInput(evt);
        assert<size_t>::equals("reportInput() call 3", 3, reportWitness);
    }
    {
        DecouplingEvent evt{};
        evt.rawInputBitmap.low = BMP2(ALT, BUTTON1);
        internals::inputHub::onRawInput(evt);
        assert<size_t>::equals("reportInput() call 4", 4, reportWitness);
    }
    {
        // Release ALT+Button1
        DecouplingEvent evt{};
        internals::inputHub::onRawInput(evt);
    }
    reportWitness = 0;
    {
        DecouplingEvent evt{};
        evt.rawInputBitmap.low = BMP(CYCLE);
        internals::inputHub::onRawInput(evt);
        assert<size_t>::equals(
            "unexpected reportInput() call", 0, reportWitness); // not called
    }
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------
//-------------------------------------------------------------------

int main()
{
    InputService::inject(&inputMock);
    InputNumber::bookAll();
    DeviceCapabilities::setFlag(DeviceCapability::CLUTCH_ANALOG);
    OnBitePoint::subscribe(bitePointCallback);
    inputHub::clutch::inputs(L_CLUTCH, R_CLUTCH);
    inputHub::clutch::cmdRecalibrateAxisInputs({RECALIBRATE1, RECALIBRATE2});
    inputHub::clutch::bitePointInputs(BITE_POINT_UP, BITE_POINT_DOWN);
    inputHub::clutch::cycleWorkingModeInputs({CYCLE});
    inputHub::altButtons::inputs({ALT});
    internals::inputHub::getReady();
    OnStart::notify();

    assert<bool>::equals("Initial state", false, inputMock.recalibrateWitness);

    TG_axis_recalibration();
    TG_bite_point();
    TG_map();
    TG_hid();

    return 0;
}
