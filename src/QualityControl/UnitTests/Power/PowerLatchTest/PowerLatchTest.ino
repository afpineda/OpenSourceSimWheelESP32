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

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include "Testing.hpp"
#include "HAL.hpp"

#include <HardwareSerial.h>

//-------------------------------------------------------
// Mocks
//-------------------------------------------------------

//-------------------------------------------------------
// Entry point
//-------------------------------------------------------

void setup()
{
  Serial.begin(115200);
  Serial.println("--READY--");

  power::configurePowerLatch(TEST_LATCH_PIN, TEST_LATCH_MODE, TEST_LATCH_DELAY);
  internals::power::getReady();
  OnStart::notify();

  Serial.println("Going to power off in 20 seconds");
  for (int i = 20; i >= 0; i--)
  {
    Serial.print("...");
    Serial.print(i);
    DELAY_MS(1000);
  }
  Serial.println("");
  Serial.println("Power Off");
  PowerService::call::shutdown();
}

void loop()
{
  DELAY_MS(5000);
}
