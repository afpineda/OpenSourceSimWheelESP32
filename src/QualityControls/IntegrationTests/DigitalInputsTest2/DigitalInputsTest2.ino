/**
 * @file DigitalInputsTest2.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-03-04
 * @brief Integration test. See [Readme](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <HardwareSerial.h>
#include "debugUtils.h"
#include "RotaryEncoderInput.h"
#include "SimWheel.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

#define ROTARY_PUSH_BN 10
#define ROTARY_CW_BN 12
#define ROTARY_CCW_BN 13

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

void inputHub::onRawInput(
    inputBitmap_t rawInputBitmap,
    inputBitmap_t rawInputChanges,
    clutchValue_t leftAxis,
    clutchValue_t rightAxis,
    bool axesChanged)
{
    Serial.print("STATE : ");
    debugPrintBool(rawInputBitmap);
    Serial.println("");
    Serial.print("CHANGE: ");
    debugPrintBool(rawInputChanges);
    Serial.println("");
}

uint8_t RotaryEncoderInput::pulseMultiplier = 1;

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    Serial.println("-- READY (DigitalInputsTest2) --");

    auto &inputSpec1 = inputs::addPCF8574Digital(PCF8574_I2C_ADDR3);
    setDebugInputNumbers(inputSpec1);
    auto &inputSpec2 = inputs::addMCP23017Digital(MCP23017_I2C_ADDR3);
    setDebugInputNumbers(inputSpec2);
    auto &inputSpec3 =
        inputs::addShiftRegisters(
            TEST_SR_SERIAL,
            TEST_SR_LOAD,
            TEST_SR_NEXT,
            TEST_SR_BUTTONS_COUNT);
    setDebugInputNumbers(inputSpec3);

    inputs::start();
    Serial.println("-- GO --");
}

void loop()
{
    delay(5000);
}