/**
 * @file BatteryCalibrationTest.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-23
 * @brief Unit test
 *
 * @copyright Licensed under the EUPL
 *
 */

//------------------------------------------------------------------
// Imports
//------------------------------------------------------------------

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalTypes.hpp"
#include "InternalServices.hpp"
#include "cd_ci_assertions.hpp"
#include <iostream>

//-------------------------------------------------------
// Globals
//-------------------------------------------------------

#define TEST_DATA_COUNT 32

int testReadings[TEST_DATA_COUNT] = {
    64,
    129,
    259,
    455,
    600,
    646,
    890,
    910,
    1099,
    1167,
    1280,
    1535,
    1641,
    1666,
    1917,
    2033,
    2100,
    2194,
    2333,
    2435,
    2564,
    2698,
    2906,
    3044,
    3190,
    3216,
    3355,
    3576,
    3677,
    3800,
    3910,
    4003};

int batteryLevel[TEST_DATA_COUNT] = {
    0,
    0,
    0,
    1,
    2,
    2,
    5,
    5,
    7,
    9,
    10,
    15,
    17,
    18,
    24,
    27,
    29,
    32,
    36,
    39,
    44,
    49,
    57,
    62,
    69,
    70,
    76,
    87,
    92,
    98,
    100,
    100};

int samplesCount[TEST_DATA_COUNT] = {
    0,
    2,
    4,
    6,
    8,
    10,
    12,
    14,
    16,
    18,
    20,
    22,
    24,
    26,
    28,
    30,
    32,
    34,
    36,
    38,
    40,
    42,
    44,
    46,
    48,
    50,
    52,
    54,
    56,
    58,
    0,
    0};

//------------------------------------------------------------------
// MOCK
//------------------------------------------------------------------

bool saved = false;

void save_callback(UserSetting setting)
{
    saved = true;
}

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

void checkCalibrationData()
{
    for (int i = 0; i < TEST_DATA_COUNT; i++)
    {
        int l = BatteryCalibrationService::call::getBatteryLevel(testReadings[i]);
        assert((l >= 0) && "Unexpected result from getBatteryLevel() with calibration data");
        assert<int>::almostEquals("Battery level mismatch", batteryLevel[i], l, 1);
    }
}

void checkCalibrationDataIsClear()
{
    for (int i = 0; i < 4096; i++)
    {
        int l = BatteryCalibrationService::call::getBatteryLevel(i);
        assert((l < 0) && "Unexpected result from getBatteryLevel() without calibration data");
    }
}

void checkAutoCalibration(int expected)
{
    int actual = BatteryCalibrationService::call::getAutoCalibrationParameter();
    assert<int>::equals("Auto-calibration mismatch", expected, actual);
}

void printTestHeader(int testNumber)
{
    std::cout << "- Test " << testNumber << " -" << std::endl;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
// Entry point
//------------------------------------------------------------------
//------------------------------------------------------------------

int main()
{
    int testnumber = 1;
    int v, l1, l2, l3, l4, l5;
    internals::batteryCalibration::getReady();
    SaveSetting::subscribe(save_callback);
    OnStart::notify();

    // Check that calibration data is properly cleared
    printTestHeader(testnumber++); // #1
    internals::batteryCalibration::addSample(1000);
    internals::batteryCalibration::clear();
    checkCalibrationDataIsClear();

    // Feed test data
    printTestHeader(testnumber++); // #2
    for (int i = 0; i < TEST_DATA_COUNT; i++)
    {
        for (int j = 0; j < samplesCount[i]; j++)
            internals::batteryCalibration::addSample(testReadings[i]);
    }

    // Test result of calibration samples
    printTestHeader(testnumber++); // #3
    checkCalibrationData();

    // Test out of range readings
    printTestHeader(testnumber++); // #4
    v = BatteryCalibrationService::call::getBatteryLevel(-1000);
    assert<int>::almostEquals("Check 1", 0, v, 1);
    v = BatteryCalibrationService::call::getBatteryLevel(0);
    assert<int>::almostEquals("Check 2", 0, v, 1);
    v = BatteryCalibrationService::call::getBatteryLevel(5000);
    assert<int>::almostEquals("Check 3", 100, v, 1);

    // Test that calibration data is stored
    printTestHeader(testnumber++); // #5
    saved = false;
    BatteryCalibrationService::call::setCalibrationData(0, 1, true);
    assert<bool>::equals("Save data", true, saved);
    internals::batteryCalibration::clear();

    // Test autocalibration
    printTestHeader(testnumber++); // #6
    BatteryCalibrationService::call::restartAutoCalibration();
    v = BatteryCalibrationService::call::getAutoCalibrationParameter();
    assert<int>::equals("Restart autocalibration algorithm", -1, v);

    // Out of range readings
    printTestHeader(testnumber++); // #7
    v = BatteryCalibrationService::call::getBatteryLevelAutoCalibrated(-100);
    assert<int>::almostEquals("Check 1", 0, v, 1);
    v = BatteryCalibrationService::call::getBatteryLevelAutoCalibrated(0);
    assert<int>::almostEquals("Check 2", 0, v, 1);
    v = BatteryCalibrationService::call::getBatteryLevelAutoCalibrated(7000);
    assert<int>::almostEquals("Check 3", 100, v, 1);

    // Autocalibration: increasing reading
    printTestHeader(testnumber++);                                             // #8
    l1 = BatteryCalibrationService::call::getBatteryLevelAutoCalibrated(2551); // First autocalibration
    checkAutoCalibration(2551);
    assert<int>::almostEquals("1st autocal", 100, l1, 1);
    l2 = BatteryCalibrationService::call::getBatteryLevelAutoCalibrated(2371); // no autocalibration
    checkAutoCalibration(2551);
    assert((l2 < 100) && "Unexpected l2");

    printTestHeader(testnumber++);                                             // #9
    l3 = BatteryCalibrationService::call::getBatteryLevelAutoCalibrated(3000); // Second autocalibration
    checkAutoCalibration(3000);
    assert<int>::almostEquals("2nd autocal", 100, l3, 1);
    l4 = BatteryCalibrationService::call::getBatteryLevelAutoCalibrated(2551); // no autocalibration
    checkAutoCalibration(3000);
    l5 = BatteryCalibrationService::call::getBatteryLevelAutoCalibrated(2371); // no autocalibration
    checkAutoCalibration(3000);
    assert((l4 < 100) && "Unexpected l4");
    assert((l4 > l5) && "ERROR: l4<=l5");
    assert((l2 > l5) && "ERROR: level(2371) calibrated to 2551 is lesser than level(2371) calibrated to 3000");

    // Test storage of auto-calibration
    printTestHeader(testnumber++); // #10
    saved = false;
    internals::batteryCalibration::clear();
    v = BatteryCalibrationService::call::getBatteryLevelAutoCalibrated(2371);
    assert<bool>::equals("Save data", true, saved);
}
