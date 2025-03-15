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
uint64_t mapBitmapWitness = 0ULL;

void internals::inputMap::map(
    bool isAltModeEngaged,
    uint64_t firmware_bitmap,
    uint64_t &low,
    uint64_t &high)
{
    mapWitnessCount++;
    mapBitmapWitness = firmware_bitmap;
    low = 0ULL;
    high = 0ULL;
    if (firmware_bitmap < 64)
        low = firmware_bitmap;
    else
        high = firmware_bitmap;
}

void inputMap::set(
    InputNumber firmware_defined,
    UserInputNumber user_defined,
    UserInputNumber user_define_alt_engaged) {}

//-------------------------------------------------------------------

size_t reportWitness = 0;

bool internals::hid::isConnected() { return true; }
bool internals::hid::supportsCustomHardwareID() { return true; }
void internals::hid::reportChangeInConfig() {}
void internals::hid::reportBatteryLevel(int batteryLevel) {}
void internals::hid::reportInput(
    uint64_t inputsLow,
    uint64_t inputsHigh,
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
    uint16_t productID) {}

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
    DecouplingEvent evt;
    evt.leftAxisValue = 0;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap = BMP(RECALIBRATE1);
    evt.rawInputChanges = evt.rawInputBitmap;
    internals::inputHub::onRawInput(evt);
    assert<bool>::equals("Unwanted axis recalibration 1", false, inputMock.recalibrateWitness);

    evt.leftAxisValue = 0;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap = BMP(RECALIBRATE2);
    evt.rawInputChanges = evt.rawInputBitmap;
    internals::inputHub::onRawInput(evt);
    assert<bool>::equals("Unwanted axis recalibration 2", false, inputMock.recalibrateWitness);

    evt.leftAxisValue = 0;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap = BMP2(RECALIBRATE1, RECALIBRATE2);
    evt.rawInputChanges = evt.rawInputBitmap;
    internals::inputHub::onRawInput(evt);
    assert<bool>::equals("Recalibration request", true, inputMock.recalibrateWitness);

    evt.leftAxisValue = 0;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap = BMP2(RECALIBRATE1, RECALIBRATE2);
    evt.rawInputChanges = 0;
    inputMock.recalibrateWitness = false;
    internals::inputHub::onRawInput(evt);
    assert<bool>::equals("Recalibration request retrigger", false, inputMock.recalibrateWitness);

    evt.leftAxisValue = 0;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap = 0b0000;
    evt.rawInputChanges = BMP2(RECALIBRATE1, RECALIBRATE2);
    internals::inputHub::onRawInput(evt);
    assert<bool>::equals("Unwanted axis recalibration 3", false, inputMock.recalibrateWitness);
}

/**
 * @brief Test the bite point event is triggered
 *
 */
void TG_bite_point()
{
    DecouplingEvent evt;
    bitePointWitness = 0;
    InputHubService::call::setBitePoint(CLUTCH_DEFAULT_VALUE);
    assert<int>::equals("Bite point initial state and callback", CLUTCH_DEFAULT_VALUE, bitePointWitness);

    evt.leftAxisValue = 254;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap = BMP(BITE_POINT_UP); // Btn 4 press
    evt.rawInputChanges = evt.rawInputBitmap;
    internals::inputHub::onRawInput(evt);
    evt.rawInputBitmap = 0ULL;
    evt.rawInputChanges = BMP(BITE_POINT_UP); // Btn 4 release
    internals::inputHub::onRawInput(evt);
    assert<int>::more("Bite point up callback", CLUTCH_DEFAULT_VALUE, bitePointWitness);

    InputHubService::call::setBitePoint(CLUTCH_DEFAULT_VALUE);
    evt.leftAxisValue = 254;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap = BMP(BITE_POINT_DOWN); // Btn 5 press
    evt.rawInputChanges = evt.rawInputBitmap;
    internals::inputHub::onRawInput(evt);
    evt.rawInputBitmap = 0ULL;
    evt.rawInputChanges = BMP(BITE_POINT_DOWN); // Btn 5 release
    internals::inputHub::onRawInput(evt);
    assert<int>::less("Bite point down callback", CLUTCH_DEFAULT_VALUE, bitePointWitness);
}

/**
 * @brief Check that inputMap is called
 *
 */
void TG_map()
{
    DecouplingEvent evt;
    evt.leftAxisValue = 0;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap = 0;
    evt.rawInputChanges = 0;
    mapWitnessCount = 0;
    internals::inputHub::onRawInput(evt);
    assert<size_t>::equals("map() call 1", 1, mapWitnessCount);

    evt.leftAxisValue = 0;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap = BMP(BUTTON1);
    evt.rawInputChanges = evt.rawInputBitmap;
    internals::inputHub::onRawInput(evt);
    assert<size_t>::equals("map() call 2", 2, mapWitnessCount);
    assert<uint64_t>::equals("map() call 2 parameter", BMP(BUTTON1), mapBitmapWitness);

    evt.leftAxisValue = 0;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap = BMP(ALT);
    evt.rawInputChanges = evt.rawInputBitmap;
    internals::inputHub::onRawInput(evt);
    assert<size_t>::equals("map() call 3", 3, mapWitnessCount);
    assert<uint64_t>::equals("map() call 3 parameter", 0ULL, mapBitmapWitness);

    evt.leftAxisValue = 0;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap = BMP(CYCLE);
    evt.rawInputChanges = evt.rawInputBitmap;
    internals::inputHub::onRawInput(evt);
    assert<size_t>::equals("unexpected map() call", 3, mapWitnessCount); // not called
}

/**
 * @brief Check that hid is called
 *
 */
void TG_hid()
{
    DecouplingEvent evt;
    evt.leftAxisValue = 0;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap = 0;
    evt.rawInputChanges = 0;
    reportWitness = 0;
    internals::inputHub::onRawInput(evt);
    assert<size_t>::equals("reportInput() call 1", 1, reportWitness);

    evt.leftAxisValue = 100;
    internals::inputHub::onRawInput(evt);
    assert<size_t>::equals("reportInput() call 2", 2, reportWitness);

    evt.rightAxisValue = 99;
    internals::inputHub::onRawInput(evt);
    assert<size_t>::equals("reportInput() call 3", 3, reportWitness);

    evt.leftAxisValue = 0;
    evt.rightAxisValue = 0;
    evt.rawInputBitmap = BMP2(ALT,BUTTON1);
    evt.rawInputChanges = evt.rawInputBitmap;
    internals::inputHub::onRawInput(evt);
    assert<size_t>::equals("reportInput() call 4", 4, reportWitness);

    evt.rawInputBitmap = BMP(CYCLE);
    evt.rawInputChanges = evt.rawInputBitmap;
    internals::inputHub::onRawInput(evt);
    assert<size_t>::equals("unexpected reportInput() call", 4, reportWitness); // not called
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
}
