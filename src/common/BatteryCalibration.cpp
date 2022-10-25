/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-04-11
 * @brief Calibrate and measure battery charge
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "Preferences.h"
#include "SimWheel.h"
//#include "debugUtils.h"

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

#define QUANTUM_BITS 5
#define QUANTUM_COUNT (1 << QUANTUM_BITS)       // = 32
#define QUANTUM_SIZE (1 << (12 - QUANTUM_BITS)) // 12 = ADC resolution

static uint16_t batteryCalibrationQuantum[QUANTUM_COUNT];
static uint32_t totalBatterySamplesCount = 0;
volatile int batteryCalibration::maxBatteryReadingEver = -1; // unknown
volatile bool batteryCalibration::calibrationInProgress = false;

//#define MIN_AUTOCALIBRATION_READING 2400
#define PREFS_NAMESPACE "bcal"
#define KEY_SAMPLE_COUNT "q%02.2d"
#define KEY_MAX_READING "m"

using namespace batteryCalibration;

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void batteryCalibration::clear()
{
    for (int q = 0; q < QUANTUM_COUNT; q++)
    {
        batteryCalibrationQuantum[q] = 0;
    }
    maxBatteryReadingEver = -1;
    totalBatterySamplesCount = 0;
    calibrationInProgress = false;
}

void batteryCalibration::begin()
{
    Preferences prefs;
    if (prefs.begin(PREFS_NAMESPACE, true))
    {
        char key[5];
        totalBatterySamplesCount = 0;
        calibrationInProgress = false;
        // Read calibration data from flash memory, or initialize
        for (int q = 0; q < QUANTUM_COUNT; q++)
        {
            snprintf(key, 5, KEY_SAMPLE_COUNT, q);
            batteryCalibrationQuantum[q] = prefs.getUShort(key, 0);
            totalBatterySamplesCount += batteryCalibrationQuantum[q];
        }
        maxBatteryReadingEver = prefs.getInt(KEY_MAX_READING, -1);
        prefs.end();
    }
    else
        clear();
}

// ----------------------------------------------------------------------------
// Save to flash memory
// ----------------------------------------------------------------------------

void batteryCalibration::save()
{
    Preferences prefs;
    if (prefs.begin(PREFS_NAMESPACE, false))
    {
        char key[5];
        for (int q = 0; q < QUANTUM_COUNT; q++)
        {
            snprintf(key, 5, KEY_SAMPLE_COUNT, q);
            prefs.putUShort(key, batteryCalibrationQuantum[q]);
        }
        prefs.end();
    }
    calibrationInProgress = false;
}

// ----------------------------------------------------------------------------
// Sampling
// ----------------------------------------------------------------------------

void batteryCalibration::addSample(int reading)
{
    if ((reading < 0) || (reading > 4095))
    {
        log_e("parameter out of range: batteryCalibration::addSample(%d)", reading);
        return;
    }

    if (!calibrationInProgress)
    {
        clear();
        calibrationInProgress = true;
    }

    int quantumIndex = reading >> (12 - QUANTUM_BITS);
    if (quantumIndex < QUANTUM_COUNT)
    {
        batteryCalibrationQuantum[quantumIndex] += 1;
        totalBatterySamplesCount++;
    }
    else
    {
        log_e("Logic error at batteryCalibration::addSample()");
        abort();
    }
}

int batteryCalibration::getCalibration(uint8_t index)
{
    if (index < QUANTUM_COUNT)
        return batteryCalibrationQuantum[index];
    else
        return -1;
}

void batteryCalibration::restoreCalibrationData(const uint16_t data[])
{
    for (uint8_t q = 0; q < QUANTUM_COUNT; q++)
    {
        batteryCalibrationQuantum[q] = data[q];
    }
}

// ----------------------------------------------------------------------------
// Interpolation
// ----------------------------------------------------------------------------

int batteryCalibration::getBatteryLevel(int reading)
{
    if (calibrationInProgress || (totalBatterySamplesCount == 0))
    {
        return -1;
    }
    else if (reading >= 4096)
    {
        return 100;
    }
    else if (reading <= 0)
    {
        return 0;
    }
    else
    {
        // Interpolate
        // serialPrintf("reading = %d | QUANTUM_SIZE = %d \n",reading,QUANTUM_SIZE);
        int quantumIndex = reading >> (12 - QUANTUM_BITS);
        if (quantumIndex < QUANTUM_COUNT)
        {
            int accumulatedSampleCount = 0;
            for (int q = 0; q < quantumIndex; q++)
                accumulatedSampleCount += batteryCalibrationQuantum[q];

            // serialPrintf("quantumIndex = %d | batteryCalibrationQuantum = %u | accumulatedSampleCount = %d\n",
            //              quantumIndex, batteryCalibrationQuantum[quantumIndex], accumulatedSampleCount);

            int relativeReading = reading - (QUANTUM_SIZE * quantumIndex);
            int samples1000perUnit = (batteryCalibrationQuantum[quantumIndex] * 1000) / QUANTUM_SIZE;
            int interpolatedSamplesCount = ((relativeReading * samples1000perUnit) / 1000) + accumulatedSampleCount;
            int batteryLevel = (interpolatedSamplesCount * 100) / totalBatterySamplesCount;

            // serialPrintf("relativeReading = %d | samples1000perUnit = %d | interpolatedSamplesCount = %d | batteryLevel = %d\n",
            //              relativeReading, samples1000perUnit, interpolatedSamplesCount, batteryLevel);
            // Serial.println("");

            return batteryLevel;
        }
        else
        {
            log_e("Logic error at batteryCalibration::getBatteryLevel()");
            abort();
            return -1;
        }
    }
}

// ----------------------------------------------------------------------------
// Auto-calibrated algorithm
// ----------------------------------------------------------------------------

int batteryCalibration::getGenericLiPoBatteryLevel(int reading)
{
    int result;
    if (reading <= 2029)
    {
        result = 0;
    }
    else if (reading <= 2290)
    {
        result = ((10 * (reading - 2029)) / 261);
    }
    else if (reading <= 2402)
    {
        result = ((50 * (reading - 2290)) / 112) + 10;
    }
    else if (reading <= 2607)
    {
        result = ((40 * (reading - 2402)) / 205) + 60;
    }
    else
        result = 100;
    return result;
}

int batteryCalibration::getBatteryLevelAutoCalibrated(int reading)
{
    if (reading >= 4096)
    {
        return 100;
    }
    else if (reading <= 0)
    {
        return 0;
    }

    if (reading > maxBatteryReadingEver)
    {
        maxBatteryReadingEver = reading;
        Preferences prefs;
        if (prefs.begin(PREFS_NAMESPACE, false))
        {
            prefs.putInt(KEY_MAX_READING, maxBatteryReadingEver);
            prefs.end();
        }
    }

    if (maxBatteryReadingEver >= 0)
    {
        reading = map(reading, 0, maxBatteryReadingEver, 0, 2608);
        return batteryCalibration::getGenericLiPoBatteryLevel(reading);
    }
    else
        return UNKNOWN_BATTERY_LEVEL;
}

void batteryCalibration::restartAutoCalibration()
{
    maxBatteryReadingEver = -1;
    
    Preferences prefs;
    if (prefs.begin(PREFS_NAMESPACE, false))
    {
        prefs.putInt(KEY_MAX_READING, maxBatteryReadingEver);
        prefs.end();
    }
}