/**
 * @file adcTools.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-12-20
 * @brief Tools for reading of ADC pins
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "adcTools.h"

// ----------------------------------------------------------------------------
// Basic ADC reading
// ----------------------------------------------------------------------------

int getADCreading(int pin, adc_atten_t attenuation)
{
  int8_t channel = digitalPinToAnalogChannel(pin);
  if (channel < 0)
  {
    log_e("getADCreading: GPIO %u is not ADC", pin);
    abort();
  }
  else if (channel > 9)
  {
    channel -= 10;
    int value = 0;
    ESP_ERROR_CHECK(adc2_config_channel_atten((adc2_channel_t)channel, attenuation));
    ESP_ERROR_CHECK(adc2_get_raw((adc2_channel_t)channel, ADC_WIDTH_BIT_12, &value));
    return value;
  }
  else
  {
    ESP_ERROR_CHECK(adc1_config_channel_atten((adc1_channel_t)channel, attenuation));
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
    return adc1_get_raw((adc1_channel_t)channel);
  }
}