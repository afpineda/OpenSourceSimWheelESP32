/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-25
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "SimWheel.h"
#include "debugUtils.h"

//-------------------------------------------------------
// Globals
//-------------------------------------------------------

void ui::turnOff()
{
}

//-------------------------------------------------------
// Entry point
//-------------------------------------------------------

void setup()
{
   Serial.begin(115200);
  //  while (!Serial) ;
  delay(5000);
  Serial.println("--READY--");
  power::setPowerLatch(TEST_LATCH_PIN, POWER_OFF_LOW, TEST_LATCH_DELAY);
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
