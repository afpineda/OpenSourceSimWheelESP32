/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-04-11
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "debugUtils.h"
#include "SimWheel.h"

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

//-------------------------------------------------------
// Mocks
//-------------------------------------------------------

void capabilities::setFlag(deviceCapability_t a, bool b)
{

}

//-------------------------------------------------------
// Auxiliary
//-------------------------------------------------------

extern void batteryCalibrationClearData();

void batteryCalibrationAssertEquals(int expectedBatteryLevel, int actualBatteryLevel)
{
    if ((actualBatteryLevel < expectedBatteryLevel - 1) || (actualBatteryLevel > expectedBatteryLevel + 1))
    {
        // NOTE: rounding errors are tolerated
        serialPrintf("Battery level mismatch: expected %d, got %d.\n", expectedBatteryLevel, actualBatteryLevel);
    }
}

void checkCalibrationData()
{
    for (int i = 0; i < TEST_DATA_COUNT; i++)
    {
        int l = batteryCalibration::getBatteryLevel(testReadings[i]);
        if (l < 0)
        {
            serialPrintf("Unexpected result from getBatteryLevel() when testing testReadings[%d]\n", i);
        }
        else
        {
            batteryCalibrationAssertEquals(l, batteryLevel[i]);
        }
    }
}

void checkCalibrationDataIsClear()
{
    for (int i = 0; i < 4096; i++)
    {
        int l = batteryCalibration::getBatteryLevel(i);
        if (l >= 0)
        {
            serialPrintf("Unexpected result %d from getBatteryLevel(%u) with no calibration data\n", l, i);
            return;
        }
    }
}

void checkAutoCalibration(int expected)
{
    if (batteryCalibration::maxBatteryReadingEver != expected)
    {
        serialPrintf(
            "Auto-calibration mismatch. Expected %d - got %d\n",
            expected,
            batteryCalibration::maxBatteryReadingEver);
    }
}

//-------------------------------------------------------
// Entry point
//-------------------------------------------------------

void setup()
{
    int testnumber = 1;

    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    while (!Serial)
        ;

    // Check that calibration data is properly cleared
    printTestHeader(testnumber++); // #1
    batteryCalibration::addSample(1000);
    batteryCalibration::clear();
    checkCalibrationDataIsClear();
    if (batteryCalibration::calibrationInProgress)
    {
        Serial.println("ERROR: unexpected state 1");
    }

    // Feed test data
    printTestHeader(testnumber++); // #2
    for (int i = 0; i < TEST_DATA_COUNT; i++)
    {
        for (int j = 0; j < samplesCount[i]; j++)
        {
            batteryCalibration::addSample(testReadings[i]);
            if (!batteryCalibration::calibrationInProgress)
            {
                Serial.println("ERROR: unexpected state 2");
            }
        }
    }
    batteryCalibration::calibrationInProgress = false;

    // Test result of calibration samples
    printTestHeader(testnumber++); // #3
    checkCalibrationData();

    // Test out of range readings
    printTestHeader(testnumber++); // #4
    batteryCalibrationAssertEquals(0, batteryCalibration::getBatteryLevel(-1000));
    batteryCalibrationAssertEquals(0, batteryCalibration::getBatteryLevel(0));
    batteryCalibrationAssertEquals(100, batteryCalibration::getBatteryLevel(5000));

    // Test that calibration data is stored in flash memory
    printTestHeader(testnumber++); // #5
    batteryCalibration::save();
    if (batteryCalibration::calibrationInProgress)
    {
        Serial.println("ERROR: unexpected state");
    }
    batteryCalibration::clear();
    batteryCalibration::begin();
    checkCalibrationData();

    // Test cleared data persistence
    printTestHeader(testnumber++); // #6
    batteryCalibration::clear();
    batteryCalibration::save();
    batteryCalibration::addSample(1000);
    batteryCalibration::begin();
    checkCalibrationDataIsClear();

    // Test LiPo characterization data
    printTestHeader(testnumber++); // #7
    // removed since getGenericLiPoBatteryLevel is not exposed anymore
    
    // batteryCalibrationAssertEquals(0, batteryCalibration::getGenericLiPoBatteryLevel(1000));
    // batteryCalibrationAssertEquals(8, batteryCalibration::getGenericLiPoBatteryLevel(2240));
    // batteryCalibrationAssertEquals(27, batteryCalibration::getGenericLiPoBatteryLevel(2327));
    // batteryCalibrationAssertEquals(46, batteryCalibration::getGenericLiPoBatteryLevel(2371));
    // batteryCalibrationAssertEquals(85, batteryCalibration::getGenericLiPoBatteryLevel(2532));
    // batteryCalibrationAssertEquals(89, batteryCalibration::getGenericLiPoBatteryLevel(2551));
    // batteryCalibrationAssertEquals(100, batteryCalibration::getGenericLiPoBatteryLevel(3000));

    // Test autocalibration
    printTestHeader(testnumber++); // #8
    batteryCalibration::restartAutoCalibration();
    if (batteryCalibration::maxBatteryReadingEver >= 0)
    {
        Serial.println("ERROR: unexpected state 3");
    }
    // Out of range readings
    batteryCalibrationAssertEquals(0, batteryCalibration::getBatteryLevelAutoCalibrated(-100));
    batteryCalibrationAssertEquals(0, batteryCalibration::getBatteryLevelAutoCalibrated(0));
    batteryCalibrationAssertEquals(100, batteryCalibration::getBatteryLevelAutoCalibrated(7000));

    // Autocalibration: increasing reading
    printTestHeader(testnumber++);                                    // #9
    int l1 = batteryCalibration::getBatteryLevelAutoCalibrated(2551); // First autocalibration
    checkAutoCalibration(2551);
    batteryCalibrationAssertEquals(100, l1);
    int l2 = batteryCalibration::getBatteryLevelAutoCalibrated(2371);
    checkAutoCalibration(2551);
    if (l2 >= 100)
    {
        serialPrintf("Unexpected l2: %d\n", l2);
    }

    printTestHeader(testnumber++);                                    // #10
    int l3 = batteryCalibration::getBatteryLevelAutoCalibrated(3000); // Second autocalibration
    checkAutoCalibration(3000);
    batteryCalibrationAssertEquals(100, l3);
    int l4 = batteryCalibration::getBatteryLevelAutoCalibrated(2551);
    checkAutoCalibration(3000);
    int l5 = batteryCalibration::getBatteryLevelAutoCalibrated(2371);
    checkAutoCalibration(3000);
    if (l4 >= 100)
    {
        serialPrintf("Unexpected l4: %d\n", l4);
    }
    else if (l4 <= l5)
    {
        serialPrintf("ERROR: l4<=l5: %d<=%d\n", l4, l5);
    }
    if (l2 <= l5)
    {
        serialPrintf(
            "ERROR: level(2371) calibrated to 2551 is lesser than level(2371) calibrated to 3000: %d<=%d\n",
            l2, l5);
    }

    // Test storage of auto-calibration
    printTestHeader(testnumber++); // #11
    batteryCalibration::maxBatteryReadingEver = 0;
    batteryCalibration::begin();
    checkAutoCalibration(3000);

    // Reset to avoid misbehaviour in future firmwares
    printTestHeader(testnumber++); // #12
    batteryCalibration::restartAutoCalibration();
    batteryCalibration::maxBatteryReadingEver = 3000;
    batteryCalibration::begin();
    checkAutoCalibration(-1);

    Serial.println("--END--");
}

void loop()
{
    delay(60 * 1000);
}
