/**
 * @file NimBLEimplTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <HardwareSerial.h>
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

volatile uint32_t capabilities::flags = 0x07;
volatile inputBitmap_t capabilities::availableInputs = 0b0111ULL;

void notify::connected()
{
    Serial.println("*** CONNECTED ***");
}

void notify::BLEdiscovering()
{
    Serial.println("*** DISCOVERING ***");
}

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

void power::powerOff()
{
    Serial.println("*** POWER OFF ***");
    powerSim = false;
}

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
    Serial.begin(115200);
    Serial.println("--START--");
    userSettings::altButtonsWorkingMode = true;
    userSettings::cpWorkingMode = CF_CLUTCH;
    userSettings::dpadWorkingMode = true;
    userSettings::bitePoint = CLUTCH_DEFAULT_VALUE;
    hidImplementation::begin("NimBLEimplTest", "Mamandurrio", true);
    Serial.println("--GO--");
}

//------------------------------------------------------------------

uint8_t btnIndex = 0;
clutchValue_t axis = CLUTCH_NONE_VALUE;
uint8_t battery = 99;
uint8_t POV = 0;

void loop()
{
    if (!powerSim)
    {
        // Simulate power off
        Serial.println("(Reset required)");
        for (;;)
            ;
    }

    if (!hidImplementation::isConnected())
    {
        Serial.println("(Waiting for connection)");
    }
    else
    {
        inputBitmap_t data = BITMAP(btnIndex);
        hidImplementation::reportInput(
            data,
            data,
            POV,
            axis,
            axis,
            axis);

        // Update pressed buttons
        btnIndex++;
        if (btnIndex > MAX_INPUT_NUMBER)
            btnIndex = 0;

        // Update DPAD state
        POV = POV + 1;
        if (POV > 8)
        {
            POV = 0;
            hidImplementation::reportChangeInConfig();
        }

        // Update battery info
        battery--;
        if (battery < 50)
            battery = 100;
        hidImplementation::reportBatteryLevel(battery);

        // Update analog axis values
        axis = axis + 5;
        if (axis >= CLUTCH_FULL_VALUE - 5)
            axis = CLUTCH_NONE_VALUE;
    }
    delay(1000);
}