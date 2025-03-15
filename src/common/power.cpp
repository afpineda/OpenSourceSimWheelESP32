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

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "HAL.hpp"
#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"

#include "freertos/FreeRTOS.h" // For TickType_t
#include "esp_bt.h"            // For esp_bt_controller_disable()
#include "esp_sleep.h"         // For deep sleep API
#include "esp32-hal-log.h"     // For log_e()

//-------------------------------------------------------------------
// Globals
//-------------------------------------------------------------------

// Power management
static gpio_num_t _wakeupPin = (gpio_num_t)-1;

// External power latch circuit
static gpio_num_t _latchPin = (gpio_num_t)-1;
static PowerLatchMode _latchMode = PowerLatchMode::POWER_OFF_LOW;
static TickType_t _latchDelay = 0;
#define _GROUNDED_ 0
#define _OPEN_DRAIN_ 1

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Public namespace
//-------------------------------------------------------------------
//-------------------------------------------------------------------

void power::configureWakeUp(RTC_GPIO wakeUpPin)
{
    _wakeupPin = AS_GPIO(wakeUpPin);
#ifndef CONFIG_IDF_TARGET_ESP32C3
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html
    // After wake up from sleep, IO pad used for wakeup will be configured as RTC IO.
    // Before using this pad as digital GPIO, reconfigure it.
    rtc_gpio_deinit(_wakeupPin);
#endif
}

//-------------------------------------------------------------------

void power::configurePowerLatch(
    OutputGPIO latchPin,
    PowerLatchMode mode,
    uint32_t waitMs)
{
    latchPin.reserve();

    switch (mode)
    {
    case PowerLatchMode::POWER_OPEN_DRAIN:
        internals::hal::gpio::forOutput(latchPin, _OPEN_DRAIN_, true);
        break;
    case PowerLatchMode::POWER_OFF_HIGH:
        internals::hal::gpio::forOutput(latchPin, false, false);
        break;
    case PowerLatchMode::POWER_OFF_LOW:
        internals::hal::gpio::forOutput(latchPin, true, false);
        break;
    }
    _latchDelay = pdMS_TO_TICKS(waitMs);
    _latchMode = mode;
    _latchPin = AS_GPIO(latchPin);
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Power management
//-------------------------------------------------------------------
//-------------------------------------------------------------------

void enterDeepSleep()
{
    // disable radios
    esp_bt_controller_disable();

    if (_wakeupPin != GPIO_NUM_NC)
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
        ESP_ERROR_CHECK(rtc_gpio_pullup_en(_wakeupPin));

        ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON));
        ESP_ERROR_CHECK(esp_sleep_enable_ext0_wakeup(_wakeupPin, 0));
#else
        // NOTE: NOT TESTED
        ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(
            BITMAP(_wakeupPin),
            ESP_GPIO_WAKEUP_GPIO_LOW));
#endif
    } // else reset is required for wake up
    esp_deep_sleep_start();

    // should not enter here
    log_e("power::powerOff(): Deep sleep not working");
    abort();
}

void latchPowerOff()
{
    if (_latchPin >= 0)
    {
        switch (_latchMode)
        {
        case PowerLatchMode::POWER_OPEN_DRAIN:
            ESP_ERROR_CHECK(gpio_set_level(_latchPin, _GROUNDED_));
            break;
        case PowerLatchMode::POWER_OFF_HIGH:
            ESP_ERROR_CHECK(gpio_set_level(_latchPin, 1));
            break;
        case PowerLatchMode::POWER_OFF_LOW:
            ESP_ERROR_CHECK(gpio_set_level(_latchPin, 0));
            break;
        }
        if (_latchDelay > 0)
            vTaskDelay(_latchDelay);
    }
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Internals
//-------------------------------------------------------------------
//-------------------------------------------------------------------

class PowerServiceProvider : public PowerService
{
public:
    virtual void shutdown() override
    {
        // Turn off peripherals
        OnShutdown::notify();
        // try external latch circuit
        latchPowerOff();
        // if still up and running, enter deep sleep
        enterDeepSleep();
    }
};

void internals::power::getReady()
{
    PowerService::inject(new PowerServiceProvider());
}
