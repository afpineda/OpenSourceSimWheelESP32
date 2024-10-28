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

#ifndef __ADC_TOOLS_H__
#define __ADC_TOOLS_H__

#include "esp_adc/adc_oneshot.h"

/**
 * @brief Get the mean of some continuous ADC readings.
 *
 * @param pin Pin number. Must be ADC-capable.
 * @param attenuation ADC Attenuation.
 * @param sampleCount Number of continuous ADC samples.
 *                    Pass 1 for a single reading (default).
 *                    Must be greater than zero.
 * @return int Mean of all continuous ADC samples or
 *             -1 if @p sampleCount is not greater than zero.
 */
int getADCreading(int pin, adc_atten_t attenuation, int sampleCount = 1);

#endif