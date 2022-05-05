/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-01
 * @brief Integration test. See [Readme](./README.md)
 * 
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 * 
 */

#include <Arduino.h>
#include "Simwheel.h"

// Use this app for testing:
// http://www.planetpointy.co.uk/joystick-test-application/

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

//------------------------------------------------------------------
// mocks
//------------------------------------------------------------------

void ui::showConnectedNotice()
{
    Serial.println("*** CONNECTED ***");
}

void ui::showBLEDiscoveringNotice()
{
    Serial.println("*** DISCOVERING ***");
}

void ui::turnOff()
{

}

void batteryCalibration::restartAutoCalibration()
{
    Serial.println("Battery autocalibration restarted");
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("--START--");
    hidImplementation::begin("UARTTest", "Mamandurrio", false, true);
    Serial.println("--GO--");
}

void loop()
{
    if (uartServer::gear > ' ')
    {
        Serial.print("GEAR: ");
        Serial.print(uartServer::gear);
        Serial.print(" RPM%: ");
        Serial.print(uartServer::rpmPercent);
        Serial.print(" SPEED: ");
        Serial.print(uartServer::speed);
        Serial.print(" MAP: ");
        Serial.print(uartServer::engineMap);
        Serial.print(" ABS: ");
        Serial.print(uartServer::absLevel);
        Serial.print(" TC: ");
        Serial.println(uartServer::tcLevel);
        uartServer::gear = ' ';
    }
    delay(1000);
}