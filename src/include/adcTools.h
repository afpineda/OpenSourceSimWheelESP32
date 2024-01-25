/**
 * @file adcTools.h
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-12-20
 * @brief Tools for reading of ADC pins
 *
 * @copyright Licensed under the EUPL
 *
 */

#ifndef __ADCTOOLS_H__
#define __ADCTOOLS_H__

#include "esp32-hal-gpio.h"
#include "esp32-hal-adc.h"
#include "driver/adc.h"

int getADCreading(int pin, adc_atten_t attenuation);

#endif

