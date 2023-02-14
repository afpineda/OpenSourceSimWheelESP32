/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-04-12
 * @brief Dummy implementation of the `batteryCalibration` namespace
 * 
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *  
 */

#include "SimWheel.h"

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

volatile bool batteryCalibration::calibrationInProgress = false;

void batteryCalibration::begin()
{

}

void batteryCalibration::clear()
{

}

void addSample(int reading, bool save)
{

}

int batteryCalibration::getCalibration(uint8_t index)
{
    return 0;
}

void batteryCalibration::restoreCalibrationData(const uint16_t data[])
{

}

void batteryCalibration::save()
{

}

int batteryCalibration::getBatteryLevel(int reading)
{
    return -1;
}

void batteryCalibration::restartAutoCalibration()
{

}

int batteryCalibration::getBatteryLevelAutoCalibrated(int reading)
{
    return 66;
}