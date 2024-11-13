/**
 * @file TelemetryDisplayTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-09-20
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include <HardwareSerial.h>
#include "SimWheel.h"
#include "SerialNotification.h"
// #include <esp_task_wdt.h>

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

int period = 0;

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

volatile clutchValue_t userSettings::bitePoint = (clutchValue_t)0;

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

int readMenuOption()
{
    char option;
    int result = -1;
    while (result < 0)
        if (Serial.available())
        {
            option = Serial.read();
            if (option == '1')
                result = 1;
            else if (option == '2')
                result = 2;
            else if (option == '3')
                result = 3;
        };
    return result;
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    Serial.println("Select option...");
    Serial.println("1 = Low data rate");
    Serial.println("2 = Medium data rate");
    Serial.println("3 = High data rate");
    int menu = readMenuOption();
    if (menu == 1)
        period = 4000;
    else if (menu == 2)
        period = 1000;
    else if (menu == 3)
        period = 250;
    else
    {
        // Should not enter here
        log_e("Logic error");
        for (;;)
            ;
    }
    notify::begin({new SerialTelemetryDisplay()}, 1);
}

//------------------------------------------------------------------

void loop()
{
    notify::telemetryData.powertrain.rpm = notify::telemetryData.powertrain.rpm + 1;
    notify::telemetryData.powertrain.speed = notify::telemetryData.powertrain.speed + 1;
    notify::telemetryData.frameID = notify::telemetryData.frameID + 1;
    delay(period);
}