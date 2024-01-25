/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Dummy implementation of the `power` namespace
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.h"

void power::begin(const gpio_num_t wakeUpPins[], const uint8_t wakeUpPinCount, bool AnyHighOrAllLow)
{

}


void power::begin(const gpio_num_t wakeUpPin, bool wakeUpHighOrLow)
{

}


void power::setPowerLatch(gpio_num_t latchPin, powerLatchMode_t mode, uint32_t waitMs)
{

}

void power::powerOff() {
    for(;;);
}

int power::getBatteryReadingForTesting(gpio_num_t battENPin, gpio_num_t battREADPin)
{
    return 4096;
}

void power::startBatteryMonitor(
    gpio_num_t battENPin,
    gpio_num_t battREADPin,
    bool testing)
{

}

int power::getLastBatteryLevel()
{
  return 66;
}
