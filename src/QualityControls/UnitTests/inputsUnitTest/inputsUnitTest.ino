/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "SimWheel.h"
#include "SimWheelTypes.h"
#include "debugUtils.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

#define IN_LEFT_CP 0
#define IN_RIGHT_CP 1
#define MASK 0ULL
#define AXIS_LEFT_BMP 0b00100
#define AXIS_RIGHT_BMP 0b01000

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

void inputHub::onStateChanged(inputBitmap_t globalState, inputBitmap_t changes)
{
    debugPrintBool(globalState);
    serialPrintf("\n   L: %d R: %d C: %d\n", clutchState::leftAxis, clutchState::rightAxis, clutchState::combinedAxis);
}

void hidImplementation::reportChangeInConfig()
{
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("-- READY --");
    inputs::begin();
    inputs::setDigitalClutchPaddles(IN_LEFT_CP, IN_RIGHT_CP);
    inputs::start();
    Serial.println("-- GO --");

    Serial.println("---- AXIS function ---");
    clutchState::setFunction(CF_AXIS);

    Serial.println("-- Analog CP");
    inputs::notifyInputEventForTesting(0, AXIS_LEFT_BMP, ~AXIS_LEFT_BMP, CLUTCH_1_4_VALUE);
    inputs::notifyInputEventForTesting(1, AXIS_RIGHT_BMP, ~AXIS_RIGHT_BMP, CLUTCH_3_4_VALUE);
    inputs::notifyInputEventForTesting(0, AXIS_LEFT_BMP, ~AXIS_LEFT_BMP, CLUTCH_NONE_VALUE);
    inputs::notifyInputEventForTesting(1, AXIS_RIGHT_BMP, ~AXIS_RIGHT_BMP, CLUTCH_NONE_VALUE);

    delay(500);
    Serial.println("-- Digital CP");
    inputs::notifyInputEvent(MASK, 0b10001);
    inputs::notifyInputEvent(MASK, 0b10011);
    inputs::notifyInputEvent(MASK, 0b00010);
    inputs::notifyInputEvent(MASK, 0b00000);

    delay(500);
    Serial.println("---- BUTTON function ---");
    clutchState::setFunction(CF_BUTTON);
    
    Serial.println("-- Analog CP");
    inputs::notifyInputEventForTesting(0, AXIS_LEFT_BMP, ~AXIS_LEFT_BMP, CLUTCH_1_4_VALUE);
    inputs::notifyInputEventForTesting(1, AXIS_RIGHT_BMP, ~AXIS_RIGHT_BMP, CLUTCH_3_4_VALUE);
    inputs::notifyInputEventForTesting(0, AXIS_LEFT_BMP, ~AXIS_LEFT_BMP, CLUTCH_NONE_VALUE);
    inputs::notifyInputEventForTesting(1, AXIS_RIGHT_BMP, ~AXIS_RIGHT_BMP, CLUTCH_NONE_VALUE);

    delay(500);
    Serial.println("-- Digital CP");
    inputs::notifyInputEvent(MASK, 0b10001);
    inputs::notifyInputEvent(MASK, 0b10011);
    inputs::notifyInputEvent(MASK, 0b00010);
    inputs::notifyInputEvent(MASK, 0b00000);

    delay(500);
    Serial.println("-- END --");
}

void loop()
{
    delay(2000);
}