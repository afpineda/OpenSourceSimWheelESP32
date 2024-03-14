/**
 * @file I2CExpanderDeepSleepTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-03-08
 * @brief Integration Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <HardwareSerial.h>
#include "SimWheel.h"
#include "debugUtils.h"

//-------------------------------------------------------
// Globals
//-------------------------------------------------------

//-------------------------------------------------------
// Mocks
//-------------------------------------------------------

void notify::powerOff()
{
}

//-------------------------------------------------------
// Auxiliary
//-------------------------------------------------------

bool print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wake up caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wake up caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wake up caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wake up caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wake up caused by ULP program");
    break;
  default:
    Serial.printf("Wake up was not caused by deep sleep: %d\n", wakeup_reason);
    break;
  }
  return (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) || (wakeup_reason == ESP_SLEEP_WAKEUP_EXT1);
}

// #define TEST_POWER_PIN GPIO_NUM_14

//-------------------------------------------------------
// Entry point
//-------------------------------------------------------

void setup()
{
  esp_log_level_set("*", ESP_LOG_ERROR);
  Serial.begin(115200);
  Serial.println("--START (I2CExpanderDeepSleepTest) --");

  print_wakeup_reason();

  inputs::addPCF8574Digital(PCF8574_I2C_ADDR3);
  inputs::addMCP23017Digital(MCP23017_I2C_ADDR3);

  Serial.println("");
  Serial.printf("Using GPIO %d as wake up source. Wake up when LOW",TEST_POWER_PIN);
  Serial.println("");

  Serial.println("Please, wait...");
  delay(2000);
  Serial.println("Entering deep sleep mode");
  power::begin(TEST_POWER_PIN);
  power::powerOff();
}

void loop()
{
  Serial.println("** Deep sleep FAILED **");
  delay(5000);
}
