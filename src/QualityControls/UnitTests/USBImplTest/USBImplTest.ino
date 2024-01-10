/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-10-24
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

//------------------------------------------------------------------
// mocks
//------------------------------------------------------------------

volatile uint32_t capabilities::flags = 0x07;

void notify::connected()
{
    Serial.println("*** CONNECTED ***");
}

// void notify::BLEdiscovering()
// {
//     Serial.println("*** DISCOVERING ***");
// }

void notify::bitePoint(clutchValue_t a)
{

}

void inputs::recalibrateAxes()
{
    Serial.println("CMD: recalibrate axes");
}

void inputs::update()
{

}

void batteryCalibration::restartAutoCalibration()
{
    Serial.println("CMD: recalibrate battery");
}

// void power::powerOff()
// {

// }

int power::getLastBatteryLevel()
{
    return UNKNOWN_BATTERY_LEVEL;
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    hidImplementation::begin("USBimplTest", "Mamandurrio", false);
    clutchState::setCPWorkingMode(CF_CLUTCH);
    clutchState::setALTButtonsWorkingMode(true);
    clutchState::setBitePoint(CLUTCH_DEFAULT_VALUE);
}

uint64_t data = 0;
clutchValue_t axis = CLUTCH_NONE_VALUE;
uint8_t battery = 99;
bool alt = false;
uint8_t POV = 0;

void loop()
{
    if (!hidImplementation::isConnected())
    {
        Serial.println("(Waiting for connection)");
    }
    else
    {
        hidImplementation::reportInput(data, alt, POV);

        data = data + 1;
        POV = POV + 1;
        if (POV > 8)
        {
            POV = 0;
            hidImplementation::reportChangeInConfig();
        }

        alt = !alt;

        axis = axis + 5;
        if (axis >= CLUTCH_FULL_VALUE - 5)
            axis = CLUTCH_NONE_VALUE;
        clutchState::leftAxis = axis;
        clutchState::rightAxis = axis;
        clutchState::combinedAxis = axis;
    }
    delay(1000);
}