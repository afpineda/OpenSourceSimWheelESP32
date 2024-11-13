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
#include "esp32-hal-log.h"

// ----------------------------------------------------------------------------
// Basic ADC reading
// ----------------------------------------------------------------------------

int getADCreading(int pin, adc_atten_t attenuation, int sampleCount)
{
  adc_channel_t channel;
  adc_unit_t adc_unit;
  if (adc_oneshot_io_to_channel(pin, &adc_unit, &channel) != ESP_OK)
  {
    log_e("getADCreading: GPIO %u is not ADC", pin);
    abort();
  }
  else if (sampleCount > 0)
  {
    adc_oneshot_unit_handle_t handle;
    adc_oneshot_unit_init_cfg_t unitCfg =
        {
            .unit_id = adc_unit,
            .clk_src = adc_oneshot_clk_src_t::ADC_RTC_CLK_SRC_DEFAULT,
            .ulp_mode = ADC_ULP_MODE_DISABLE,
        };
    adc_oneshot_chan_cfg_t channelCfg =
        {
            .atten = attenuation,
            .bitwidth = ADC_BITWIDTH_12,
        };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unitCfg, &handle));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(handle, channel, &channelCfg));
    int result = 0;
    for (int i = 0; i < sampleCount; i++)
    {
      int reading;
      ESP_ERROR_CHECK(adc_oneshot_read(handle, channel, &reading));
      result += reading;
    }
    result = result / sampleCount;
    ESP_ERROR_CHECK(adc_oneshot_del_unit(handle));
    return result;
  }
  return -1;
}