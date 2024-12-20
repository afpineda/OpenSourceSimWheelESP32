/**
 * @file power.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Implementation of the `power` namespace
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <Arduino.h>
#include "SimWheel.h"
#include "adcTools.h"
#include <esp_sleep.h>
#include "esp_pm.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp32-hal-gpio.h"
// #include "esp32-hal-adc.h"
// #include "driver/adc.h"
#include "driver/rtc_io.h"

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

// Power management
static gpio_num_t wakeup_pin = GPIO_NUM_NC;

// External power latch circuit
static gpio_num_t _latchPin = (gpio_num_t)-1;
static powerLatchMode_t _latchMode = POWER_OFF_LOW;
static TickType_t latchDelay = 0;
#define _GROUNDED_ 0
#define _OPEN_DRAIN_ 1

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void power::begin(const gpio_num_t wakeUpPin)
{
  // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html
  // After wake up from sleep, IO pad used for wakeup will be configured as RTC IO.
  // Before using this pad as digital GPIO, reconfigure it.

#ifndef CONFIG_IDF_TARGET_ESP32C3
  rtc_gpio_deinit(wakeUpPin);
#endif

  if (rtc_gpio_is_valid_gpio(wakeUpPin))
    wakeup_pin = wakeUpPin;
}

// ----------------------------------------------------------------------------

void power::setPowerLatch(gpio_num_t latchPin, powerLatchMode_t mode, uint32_t waitMs)
{
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.pin_bit_mask = (1ULL << latchPin);
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  if (GPIO_IS_VALID_OUTPUT_GPIO(latchPin))
  {
    // Keep power on
    switch (mode)
    {
    case POWER_OPEN_DRAIN:
      io_conf.mode = GPIO_MODE_OUTPUT_OD;
      ESP_ERROR_CHECK(gpio_config(&io_conf));
      ESP_ERROR_CHECK(gpio_set_level(latchPin, _OPEN_DRAIN_));
      break;
    case POWER_OFF_HIGH:
      io_conf.mode = GPIO_MODE_OUTPUT;
      ESP_ERROR_CHECK(gpio_config(&io_conf));
      ESP_ERROR_CHECK(gpio_set_level(latchPin, 0));
      break;
    case POWER_OFF_LOW:
      io_conf.mode = GPIO_MODE_OUTPUT;
      ESP_ERROR_CHECK(gpio_config(&io_conf));
      ESP_ERROR_CHECK(gpio_set_level(latchPin, 1));
      break;
    }
    latchDelay = waitMs / portTICK_RATE_MS;
    _latchMode = mode;
    _latchPin = latchPin;
  }
  else
  {
    log_e("power::setPowerLatch(): no valid GPIO = %u", latchPin);
    abort();
  }
}

// ----------------------------------------------------------------------------
// Power management
// ----------------------------------------------------------------------------

void enterDeepSleep()
{
  // disable radios
  esp_bt_controller_disable();

  if (wakeup_pin != GPIO_NUM_NC)
  {
    // Disable pins to avoid current drainage through pull resistors
    // except for the wake-up GPIO
    for (int p = 0; p <= GPIO_NUM_MAX; p++)
    {
      if (GPIO_IS_VALID_GPIO((gpio_num_t)p))
      {
        gpio_pulldown_dis((gpio_num_t)p);
        gpio_pullup_dis((gpio_num_t)p);
      }
#ifndef CONFIG_IDF_TARGET_ESP32C3
      if (RTC_GPIO_IS_VALID_GPIO((gpio_num_t)p))
      {
        rtc_gpio_pullup_dis((gpio_num_t)p);
        rtc_gpio_pulldown_dis((gpio_num_t)p);
      }
#endif
    }
#ifndef CONFIG_IDF_TARGET_ESP32C3
    ESP_ERROR_CHECK(rtc_gpio_pullup_en(wakeup_pin));

    ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON));
    ESP_ERROR_CHECK(esp_sleep_enable_ext0_wakeup(wakeup_pin, 0));
#else
    // NOTE: NOT TESTED
    ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(
        BITMAP(wakeup_pin),
        ESP_GPIO_WAKEUP_GPIO_LOW));
#endif
  } // else reset is required for wake up
  esp_deep_sleep_start();

  // should not enter here
  log_e("power::powerOff(): Deep sleep not working");
  delay(5000);
  ESP.restart();
}

void latchPowerOff()
{
  if (_latchPin >= 0)
  {
    switch (_latchMode)
    {
    case POWER_OPEN_DRAIN:
      ESP_ERROR_CHECK(gpio_set_level(_latchPin, _GROUNDED_));
      break;
    case POWER_OFF_HIGH:
      ESP_ERROR_CHECK(gpio_set_level(_latchPin, 1));
      break;
    case POWER_OFF_LOW:
      ESP_ERROR_CHECK(gpio_set_level(_latchPin, 0));
      break;
    }
    if (latchDelay > 0)
      vTaskDelay(latchDelay);
  }
}

void power::powerOff()
{
  // Turn off peripherals
  pixels::shutdown();
  notify::shutdown();
  // try external latch circuit
  latchPowerOff();
  // if still up and running, enter deep sleep
  enterDeepSleep();
}
