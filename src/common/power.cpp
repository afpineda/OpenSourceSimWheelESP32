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
static uint64_t ext1_wakeup_sources = 0;
static esp_sleep_ext1_wakeup_mode_t ext1_wakeup_mode = ESP_EXT1_WAKEUP_ANY_HIGH;

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
  for (uint64_t p = 0; p < GPIO_NUM_MAX; p++)
    if (BITMAP(p) & ext1_wakeup_sources)
      rtc_gpio_deinit((gpio_num_t)p);
#endif

  if (rtc_gpio_is_valid_gpio(wakeUpPin))
    ext1_wakeup_sources = BITMAP(wakeUpPin);
  else
    ext1_wakeup_sources = 0ULL;
  ext1_wakeup_mode = ESP_EXT1_WAKEUP_ALL_LOW;
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
  esp_bluedroid_disable();
  esp_bt_controller_disable();

  // Disable pins to avoid current drainage through pull resistors
  // Enable proper pull resistors for wake up (if available)
  for (int p = 0; p < GPIO_NUM_MAX; p++)
    if (GPIO_IS_VALID_GPIO((gpio_num_t)p))
    {
      uint64_t pinBitmap = BITMAP(p);
      if (pinBitmap & ext1_wakeup_sources)
      {
        if (ext1_wakeup_mode == ESP_EXT1_WAKEUP_ANY_HIGH)
        {
          gpio_pullup_dis((gpio_num_t)p);
          gpio_pulldown_en((gpio_num_t)p);
        }
        else
        {
          gpio_pulldown_dis((gpio_num_t)p);
          gpio_pullup_en((gpio_num_t)p);
        }
      }
      else
      {
        gpio_pulldown_dis((gpio_num_t)p);
        gpio_pullup_dis((gpio_num_t)p);
      }
    }

  // enter deep sleep
  if (ext1_wakeup_sources != 0ULL)
  {
    ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON));
#ifndef CONFIG_IDF_TARGET_ESP32C3
    ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup(ext1_wakeup_sources, ext1_wakeup_mode));
#else
    // NOTE: NOT TESTED
    esp_deepsleep_gpio_wake_up_mode_t aux =
        (ext1_wakeup_mode == ESP_EXT1_WAKEUP_ANY_HIGH) ? ESP_GPIO_WAKEUP_GPIO_HIGH : ESP_GPIO_WAKEUP_GPIO_LOW;
    ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(ext1_wakeup_sources, aux));
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
  notify::powerOff();
  // try external latch circuit
  latchPowerOff();
  // if still up and running, enter deep sleep
  enterDeepSleep();
}
