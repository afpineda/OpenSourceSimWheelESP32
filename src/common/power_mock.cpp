/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Dummy implementation of the `power` namespace
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.h"

void power::begin(const gpio_num_t wakeUpPin)
{
}

void power::setPowerLatch(gpio_num_t latchPin, powerLatchMode_t mode, uint32_t waitMs)
{
}

void power::powerOff()
{
    for (;;)
        ;
}