/**
 * @file DeepSleepTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include "InputHardware.hpp"
#include "Testing.hpp"
#include "HAL.hpp"

#include <vector>
#include <algorithm>
#include <HardwareSerial.h> // for Serial

//-------------------------------------------------------
// Globals
//-------------------------------------------------------

//-------------------------------------------------------
// Mocks
//-------------------------------------------------------

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
}

uint8_t getI2CFullAddress(uint8_t I2CAddress, bool isFullAddress, I2CBus bus)
{
  static bool firstRun = true;
  static std::vector<uint8_t> i2cAddressesFromProbe;

  // Retrieve all 7-bit addresses found in the bus (only once)
  if (firstRun)
  {
    firstRun = false;
    internals::hal::i2c::probe(i2cAddressesFromProbe, bus);
  }

  // Check if a device is responding to the given
  // I2C address
  uint8_t fullAddress;
  if (isFullAddress)
  {
    if (std::find(
            i2cAddressesFromProbe.begin(),
            i2cAddressesFromProbe.end(),
            I2CAddress) != i2cAddressesFromProbe.end())
      fullAddress = I2CAddress;
    else
      fullAddress = 0xFF; // Device not found
  }
  else
  {
    fullAddress = internals::hal::i2c::findFullAddress(
        i2cAddressesFromProbe,
        I2CAddress);
  }
  return fullAddress;
}

//-------------------------------------------------------
// Entry point
//-------------------------------------------------------

void setup()
{
  Serial.begin(115200);
  Serial.println("--START--");
  print_wakeup_reason();
  Serial.println("");
  Serial.printf(
      "Using GPIO %d as wake up source. Wake up when LOW.\n",
      TEST_POWER_PIN);

  // Create GPIO Expanders as they will work as a wake-up source.
  // Ignored if they are not attached to the test circuit.
  // Note that we are not testing the input hardware nor the
  // inputs subsystem.
  uint8_t pcf8574address = getI2CFullAddress(PCF8574_I2C_ADDR3, false, I2CBus::PRIMARY);
  uint8_t mcp23017address = getI2CFullAddress(MCP23017_I2C_ADDR3, false, I2CBus::PRIMARY);
  PCF8574Expander spec1;
  MCP23017Expander spec2;
  try
  {
    new PCF8574ButtonsInput(
        spec1,
        pcf8574address);
  }
  catch (...)
  {
    Serial.println("Note: PCF8574 not found");
  }

  try
  {
    new MCP23017ButtonsInput(
        spec2,
        mcp23017address);
  }
  catch (...)
  {
    Serial.println("Note: MCP23017 not found");
  }

  // Configure the power subsystem
  power::configureWakeUp(TEST_POWER_PIN);
  internals::power::getReady();
  OnStart::notify();

  Serial.println("Please, wait...");
  delay(2000);
  Serial.println("");
  Serial.println("Entering deep sleep mode");
  delay(1000);
  PowerService::call::shutdown();
}

void loop()
{
  Serial.println("** Deep sleep FAILED **");
  DELAY_MS(5000);
}
