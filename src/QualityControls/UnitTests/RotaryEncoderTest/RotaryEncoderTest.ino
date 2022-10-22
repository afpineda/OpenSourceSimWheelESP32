/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Input from rotary encoders 
 * 
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 * 
 */

#include <Arduino.h>
#include "RotaryEncoderInput.h"
#include "debugUtils.h"
#include "SimWheel.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

inputBitmap_t globalState = 0;

//------------------------------------------------------------------
// mocks
//------------------------------------------------------------------

void inputs::notifyInputEvent(inputBitmap_t mask, inputBitmap_t state)
{
    inputBitmap_t changes = globalState ^ state;
    globalState = state;
    Serial.print("MASK  : ");
    debugPrintBool(mask);
    Serial.println("");
    Serial.print("STATE : ");
    debugPrintBool(state);
    Serial.println("");
    Serial.print("CHANGE: ");
    debugPrintBool(changes);
    Serial.println("");
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    Serial.begin(115200);
    while (!Serial) ;
    Serial.println("-- READY --");
    
    // Uncomment the following line when testing alterante enconding
    // and vice-versa

    new RotaryEncoderInput(TEST_ROTARY_ALT_A,TEST_ROTARY_ALT_B,5,6,true);
    
    // Comment out the following line when not testing alternate encoding
    // and vice-versa

    new RotaryEncoderInput(TEST_ROTARY_CLK,TEST_ROTARY_DT,5);

    Serial.println("-- GO --");
}

void loop()
{
    delay(2000);
}
