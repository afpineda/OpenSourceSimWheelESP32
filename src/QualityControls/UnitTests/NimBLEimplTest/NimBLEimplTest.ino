/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Unit Test. See [README](./README.md)
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

bool powerSim = true;

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

void power::powerOff(bool forced)
{
    Serial.println("*** POWER OFF ***");
    powerSim = false;
}

void uartServer::onReceive(char *text)
{
    Serial.print("COMMAND: ");
    Serial.println(text);
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
    hidImplementation::begin("SimWheelTest", "Mamandurrio", true);
    Serial.println("--GO--");
}

#define axis1 -127
#define axis2 127

uint64_t data = 0;
clutchValue_t axis = axis1;
uint8_t battery = 99;
bool alt = false;
uint8_t POV = 0;

void loop()
{
    if (!powerSim) {
        // Simulate power off
        Serial.println("(Reset required)");
        for(;;);
    }
        
    if (!hidImplementation::isConnected())
    {
        Serial.println("(Waiting for connection)");
    }
    else
    {
        hidImplementation::reportBatteryLevel(battery);
        hidImplementation::reportInput(data,alt,axis,POV);

        data = data + 1;
        POV = POV + 1;
        if (POV>8)
            POV = 0;

        alt = !alt;

        battery--;
        if (battery<50)
            battery=100;

        axis = axis + 5;
        if (axis>=axis2)
            axis = axis1; 
    }
    delay(1000);
}