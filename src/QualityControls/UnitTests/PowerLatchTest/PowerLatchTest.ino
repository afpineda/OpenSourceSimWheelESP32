/**
 * @file PowerLatchTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-25
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <Arduino.h>
#include "SimWheel.h"
#include "debugUtils.h"

//-------------------------------------------------------
// Mocks
//-------------------------------------------------------

void notify::shutdown() {}

//-------------------------------------------------------
// Entry point
//-------------------------------------------------------

void setup()
{
  esp_log_level_set("*", ESP_LOG_ERROR);
  Serial.begin(115200);
  Serial.println("--READY--");
  power::setPowerLatch(TEST_LATCH_PIN, TEST_LATCH_MODE, TEST_LATCH_DELAY);
  Serial.println("Going to power off in 20 seconds");
  for (int i = 20; i >= 0; i--)
  {
    Serial.print("...");
    Serial.print(i);
    delay(1000);
  }
  Serial.println("");
  Serial.println("Power Off");
  power::powerOff();
}

void loop()
{
  delay(5000);
}
