/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-10-01
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "SimWheel.h"

//-------------------------------------------------------
// Globals
//-------------------------------------------------------

#define TEST_POWER_PIN GPIO_NUM_2

//-------------------------------------------------------
// Mocks
//-------------------------------------------------------

void notify::powerOff()
{
}

//-------------------------------------------------------
// Auxiliary
//-------------------------------------------------------

void print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program");
    break;
  default:
    Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    break;
  }
}

void waitForButton()
{
  while (digitalRead(TEST_POWER_PIN) == LOW)
    delay(50);
  while (digitalRead(TEST_POWER_PIN) == HIGH)
    delay(50);
}

//-------------------------------------------------------
// Entry point
//-------------------------------------------------------

void setup()
{
  esp_log_level_set("*", ESP_LOG_ERROR);
  Serial.begin(115200);
  while (!Serial)
    ;
  // delay(2000);
  Serial.println("--START--");

  print_wakeup_reason();
  power::begin(TEST_POWER_PIN, false);

  ESP_ERROR_CHECK(gpio_set_direction(TEST_POWER_PIN, GPIO_MODE_INPUT));
  ESP_ERROR_CHECK(gpio_set_pull_mode(TEST_POWER_PIN, GPIO_PULLUP_ONLY));

  Serial.println("POWER ON and running");
  Serial.println("Push POWER button to enter deep sleep mode...");
  waitForButton();
  // Serial.println("Going to sleep in 10 seconds");
  // for (int i=10; i>=0; i--) {
  //   Serial.print("...");
  //   Serial.print(i);
  //   delay(1000);
  // }
  Serial.println("");
  Serial.println("Entering deep sleep mode");
  delay(1000);
  power::powerOff();
}

void loop()
{
  Serial.println("** Deep sleep FAILED **");
  delay(5000);
}
