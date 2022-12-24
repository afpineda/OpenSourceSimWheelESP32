/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Implementation of the `power` namespace
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
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

// Battery monitor
#define _GROUNDED_ 0
#define _OPEN_DRAIN_ 1
static gpio_num_t batteryENPin = (gpio_num_t)-1;
static gpio_num_t batteryREADPin = (gpio_num_t)-1;
static TaskHandle_t batteryMonitorDaemon = nullptr;
static bool testInProgress = false;
int lastBatteryLevel = 100;

// #define BATTMON_STACK_SIZE 1536
#define BATTMON_STACK_SIZE 2048
#define BATTMON_SAMPLE_COUNT 50
#define BATTMON_SAMPLING_RATE_TICKS ((2 * 60 * 1000) / portTICK_RATE_MS) // 2 minutes
#define BATTMON_TESTING_RATE_TICKS ((5 * 1000) / portTICK_RATE_MS)       // 5 seconds
#define LOW_BATTERY_LEVEL 10                                             // percentage
#define SAFETY_BATTERY_LEVEL 4                                           // percentage

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void power::begin(const gpio_num_t wakeUpPins[], const uint8_t wakeUpPinCount, bool AnyHighOrAllLow)
{
  // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html
  // After wake up from sleep, IO pad used for wakeup will be configured as RTC IO.
  // Before using this pad as digital GPIO, reconfigure it.
  for (uint64_t p = 0; p < GPIO_NUM_MAX; p++)
    if (BITMAP(p) & ext1_wakeup_sources)
      rtc_gpio_deinit((gpio_num_t)p);

  ext1_wakeup_sources = 0ULL;
  for (uint8_t i = 0; i < wakeUpPinCount; i++)
    if (rtc_gpio_is_valid_gpio(wakeUpPins[i]))
      ext1_wakeup_sources |= wakeUpPins[i];
    else
      log_e("power::begin(): Invalid rtc pin for wakeup (%d)", wakeUpPins[i]);
  ext1_wakeup_mode = AnyHighOrAllLow ? ESP_EXT1_WAKEUP_ANY_HIGH : ESP_EXT1_WAKEUP_ALL_LOW;
}

void power::begin(
    const gpio_num_t wakeUpPin, bool wakeUpHighOrLow)
{
  for (uint64_t p = 0; p < GPIO_NUM_MAX; p++)
    if (BITMAP(p) & ext1_wakeup_sources)
      rtc_gpio_deinit((gpio_num_t)p);

  if (rtc_gpio_is_valid_gpio(wakeUpPin))
    ext1_wakeup_sources = BITMAP(wakeUpPin);
  else
    ext1_wakeup_sources = 0ULL;
  ext1_wakeup_mode = wakeUpHighOrLow ? ESP_EXT1_WAKEUP_ANY_HIGH : ESP_EXT1_WAKEUP_ALL_LOW;
}

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
  // esp_wifi_stop();

  // for (int p = 0; p < GPIO_NUM_MAX; p++)
  //   if (GPIO_IS_VALID_GPIO((gpio_num_t)p))
  //     gpio_reset_pin((gpio_num_t)p);

  // disable pins to avoid current drainage through pull resistors
  // enable proper pull resistors
  for (int p = 0; p < GPIO_NUM_MAX; p++)
    if (GPIO_IS_VALID_GPIO((gpio_num_t)p) && !((p >= 6) && (p <= 11))) // Exclude flash memory pins
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
    ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup(ext1_wakeup_sources, ext1_wakeup_mode));
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

// ----------------------------------------------------------------------------
// Battery monitor daemon
// ----------------------------------------------------------------------------

int power::getBatteryReadingForTesting(gpio_num_t battENPin, gpio_num_t battREADPin)
{

  if ((battENPin < 0) || (gpio_set_level(battENPin, 1) == ESP_OK))
  {
//    adc_power_acquire();
    vTaskDelay(200);
    uint64_t sum = 0;
    for (int i = 0; i < BATTMON_SAMPLE_COUNT; i++)
    {
      // sum += analogRead(batteryREADPin);
      sum += getADCreading(battREADPin, ADC_ATTEN_DB_11);
      // vTaskDelay(10);
    }
    if (battENPin >= 0)
      gpio_set_level(battENPin, 0);
//    adc_power_release();
    return (sum / BATTMON_SAMPLE_COUNT);
  }
  return 0;
}

// ----------------------------------------------------------------------------

void batteryDaemonLoop(void *unused)
{
  while (true)
  {
    // Determine battery level
    int lastBatteryReading = power::getBatteryReadingForTesting(batteryENPin, batteryREADPin);
    if (lastBatteryReading < 150)
    {
      // Battery(+) is not connected, so battery level is unknown
      lastBatteryLevel = UNKNOWN_BATTERY_LEVEL;
    }
    else
    {
      lastBatteryLevel = batteryCalibration::getBatteryLevel(lastBatteryReading);
      if (lastBatteryLevel < 0)
      {
        // Battery calibration is *not* available
        // fallback to auto-calibration algorithm
        lastBatteryLevel = batteryCalibration::getBatteryLevelAutoCalibrated(lastBatteryReading);
      }
    }

    // Report battery level
    hidImplementation::reportBatteryLevel(lastBatteryLevel);
    if (lastBatteryLevel <= SAFETY_BATTERY_LEVEL)
    {
      // The DevKit must go to deep sleep before battery depletes, otherwise, it keeps
      // draining current even if there is not enought voltage to turn it on.
      power::powerOff();
    }
    else if (lastBatteryLevel <= LOW_BATTERY_LEVEL)
      notify::lowBattery();

    // Delay to next sample
    if (testInProgress)
      vTaskDelay(BATTMON_TESTING_RATE_TICKS);
    else
      vTaskDelay(BATTMON_SAMPLING_RATE_TICKS);
  } // end while
}

// ----------------------------------------------------------------------------
// Configure and start battery monitor
// ----------------------------------------------------------------------------

void configureBatteryMonitor(
    gpio_num_t enableBatteryReadPin,
    gpio_num_t batteryLevelPin)
{
  if (!GPIO_IS_VALID_GPIO(batteryLevelPin) ||
      ((enableBatteryReadPin >= 0) && !GPIO_IS_VALID_OUTPUT_GPIO(enableBatteryReadPin)) ||
      (digitalPinToAnalogChannel(batteryLevelPin) < 0))
  {
    log_e("power::startBatteryMonitor(): given pins are not usable");
    abort();
  }

  gpio_config_t io_conf = {};
  if (enableBatteryReadPin >= 0)
  {
    // configure _battEN_ pin
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << enableBatteryReadPin);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_ERROR_CHECK(gpio_set_level(enableBatteryReadPin, 0));
  }

  // configure _battRead_ pin
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pin_bit_mask = (1ULL << batteryLevelPin);
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  ESP_ERROR_CHECK(gpio_config(&io_conf));

  // Store parameters
  batteryENPin = enableBatteryReadPin;
  batteryREADPin = batteryLevelPin;

  capabilities::setFlag(deviceCapability_t::CAP_BATTERY);
}

void power::startBatteryMonitor(
    gpio_num_t battENPin,
    gpio_num_t battREADPin,
    bool testing)
{
  if (batteryMonitorDaemon == nullptr)
  {
    testInProgress = testing;
    configureBatteryMonitor(battENPin, battREADPin);
    xTaskCreate(batteryDaemonLoop, "BattMon", BATTMON_STACK_SIZE, nullptr, tskIDLE_PRIORITY + 1, &batteryMonitorDaemon);
    if (batteryMonitorDaemon == nullptr)
    {
      log_e("power::startBatteryMonitor(): unable to start daemon");
      abort();
    }
    capabilities::setFlag(deviceCapability_t::CAP_BATTERY);
  }
}

// ----------------------------------------------------------------------------
// Getters
// ----------------------------------------------------------------------------

int power::getLastBatteryLevel()
{
  return lastBatteryLevel;
}
