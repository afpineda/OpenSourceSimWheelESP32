/**
 * @file batteryCalibration.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-04-11
 * @brief Everything related to battery profiling.
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
#include "HAL.hpp"

//-------------------------------------------------------------------
// Globals
//-------------------------------------------------------------------

#define QUANTUM_BITS 5
#define QUANTUM_COUNT (1 << QUANTUM_BITS)       // = 32
#define QUANTUM_SIZE (1 << (12 - QUANTUM_BITS)) // 12 = ADC resolution

static uint16_t batteryCalibrationQuantum[QUANTUM_COUNT];
static uint32_t totalBatterySamplesCount = 0;
int maxBatteryReadingEver = -1; // unknown

//-------------------------------------------------------------------
// Interpolation
//-------------------------------------------------------------------

int getGenericLiPoBatteryLevel(int reading)
{
    int result;
    if (reading < 4059)
    {
        result = 0;
    }
    else if (reading < 4580)
    {
        result = ((10 * (reading - 4059)) / 521);
    }
    else if (reading < 4803)
    {
        result = ((50 * (reading - 4580)) / 223) + 10;
    }
    else if (reading < 5213)
    {
        result = ((40 * (reading - 4803)) / 410) + 60;
    }
    else
        result = 100;
    return result;
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Service class
//-------------------------------------------------------------------
//-------------------------------------------------------------------

class BatteryCalibrationProvider : public BatteryCalibrationService
{
    virtual void restartAutoCalibration() override
    {
        maxBatteryReadingEver = -1;
        SaveSetting::notify(UserSetting::BATTERY_AUTO_CALIBRATION);
    }

    virtual int getBatteryLevel(int reading) override
    {
        if (totalBatterySamplesCount == 0)
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
            // Serial.printf("reading = %d | QUANTUM_SIZE = %d \n",reading,QUANTUM_SIZE);
            int quantumIndex = reading >> (12 - QUANTUM_BITS);
            if (quantumIndex < QUANTUM_COUNT)
            {
                int accumulatedSampleCount = 0;
                for (int q = 0; q < quantumIndex; q++)
                    accumulatedSampleCount += batteryCalibrationQuantum[q];

                // Serial.printf("quantumIndex = %d | batteryCalibrationQuantum = %u | accumulatedSampleCount = %d\n",
                //              quantumIndex, batteryCalibrationQuantum[quantumIndex], accumulatedSampleCount);

                int relativeReading = reading - (QUANTUM_SIZE * quantumIndex);
                int samples1000perUnit = (batteryCalibrationQuantum[quantumIndex] * 1000) / QUANTUM_SIZE;
                int interpolatedSamplesCount = ((relativeReading * samples1000perUnit) / 1000) + accumulatedSampleCount;
                int batteryLevel = (interpolatedSamplesCount * 100) / totalBatterySamplesCount;

                // Serial.printf("relativeReading = %d | samples1000perUnit = %d | interpolatedSamplesCount = %d | batteryLevel = %d\n",
                //              relativeReading, samples1000perUnit, interpolatedSamplesCount, batteryLevel);
                // Serial.println("");

                return batteryLevel;
            }
            else
                throw std::runtime_error("Logic error at batteryCalibration::getBatteryLevel()");
            return -1;
        }
    }

    virtual int getBatteryLevelAutoCalibrated(int reading) override
    {
        if (reading >= 4095)
        {
            return 100;
        }
        else if (reading <= 0)
        {
            return 0;
        }
        else if (reading > maxBatteryReadingEver)
        {
            maxBatteryReadingEver = reading;
            SaveSetting::notify(UserSetting::BATTERY_AUTO_CALIBRATION);
        }

        if (maxBatteryReadingEver >= 0)
        {
            int minBatteryReading =
                4059 * maxBatteryReadingEver / 5213;
            reading = map_value(
                reading,
                minBatteryReading,
                maxBatteryReadingEver,
                4059,
                5213);
            return getGenericLiPoBatteryLevel(reading);
        }
        else
            return UNKNOWN_BATTERY_LEVEL;
    }

    virtual uint8_t getCalibrationDataCount() override
    {
        return QUANTUM_COUNT;
    }

    virtual uint16_t getCalibrationData(uint8_t index) override
    {
        if (index < QUANTUM_COUNT)
            return batteryCalibrationQuantum[index];
        else
            return 0;
    }

    virtual void setCalibrationData(uint8_t index, uint16_t data, bool save) override
    {
        if ((index < QUANTUM_COUNT) && (batteryCalibrationQuantum[index] != data))
        {
            totalBatterySamplesCount -= batteryCalibrationQuantum[index];
            totalBatterySamplesCount += data;
            batteryCalibrationQuantum[index] = data;
            if (save)
                SaveSetting::notify(UserSetting::BATTERY_CALIBRATION_DATA);
        }
    }

    virtual int getAutoCalibrationParameter() override
    {
        return maxBatteryReadingEver;
    }

    virtual void setAutoCalibrationParameter(int value, bool save) override
    {
        if (value != maxBatteryReadingEver)
        {
            maxBatteryReadingEver = value;
            if (save)
                SaveSetting::notify(UserSetting::BATTERY_AUTO_CALIBRATION);
        }
    }
};

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Internals
//-------------------------------------------------------------------
//-------------------------------------------------------------------

// void internals::batteryCalibration::save()
// {
//     SaveSetting::notify(UserSetting::BATTERY_CALIBRATION_DATA);
// }

//-------------------------------------------------------------------

void internals::batteryCalibration::clear()
{
    for (int q = 0; q < QUANTUM_COUNT; q++)
        batteryCalibrationQuantum[q] = 0;
    maxBatteryReadingEver = -1;
    totalBatterySamplesCount = 0;
}

//-------------------------------------------------------------------

void internals::batteryCalibration::addSample(int reading)
{
    if ((reading < 0) || (reading > 4095))
        throw std::runtime_error("parameter out of range: batteryCalibration::addSample()");

    int quantumIndex = reading >> (12 - QUANTUM_BITS);
    if (quantumIndex < QUANTUM_COUNT)
    {
        batteryCalibrationQuantum[quantumIndex] += 1;
        totalBatterySamplesCount++;
    }
    else
        throw std::runtime_error("Logic error at batteryCalibration::addSample()");
}

//-------------------------------------------------------------------

void batteryCalibrationStart()
{
    internals::batteryCalibration::clear();
    LoadSetting::notify(UserSetting::BATTERY_AUTO_CALIBRATION);
    LoadSetting::notify(UserSetting::BATTERY_CALIBRATION_DATA);
    if (totalBatterySamplesCount > 0)
        DeviceCapabilities::setFlag(DeviceCapability::BATTERY_CALIBRATION_AVAILABLE);
}

void internals::batteryCalibration::getReady()
{
    if (!FirmwareService::call::isRunning())
    {
        BatteryCalibrationService::inject(new BatteryCalibrationProvider());
        OnStart::subscribe(batteryCalibrationStart);
    }
}
