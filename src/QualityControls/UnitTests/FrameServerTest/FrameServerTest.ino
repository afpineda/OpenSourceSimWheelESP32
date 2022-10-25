/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-08
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "SimWheel.h"
#include <esp_task_wdt.h>

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

#define OLED_BUFFER_SIZE (128 * 128) / 8 // Maximun buffer size: 128x128 pixels

uint8_t buffer[OLED_BUFFER_SIZE];

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

uint8_t *uiManager::getFrameServerBuffer()
{
    return buffer;
}

void uiManager::unsafeDisplayFrameServerBuffer()
{
    ui::display(buffer);
}

void uiManager::hide(screenPriority_t priority)
{
    ui::clear();
}

void idle(void *unused)
{
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    // while (!Serial)
    //     ;
    Serial.println("-- READY --");
    esp_task_wdt_init(30, false);
    ui::begin();
    ui::frameServerSetEnabled(true);
    Serial.println("-- GO --");
}

// Simulate received data
void loop()
{
    Serial.println("frame server enabled");
    ui::frameServerSetEnabled(true);
    uartServer::gear = 'N';
    uartServer::rpmPercent = 0;
    uartServer::speed = 0;
    uartServer::absLevel = 0;
    uartServer::engineMap = 0;
    uartServer::tcLevel = 0;
    delay(2000);
    for (int i = 0; i < 500; i++)
    {
        if (i < 100)
            uartServer::gear = '1';
        else if (i < 200)
            uartServer::gear = '2';
        else if (i < 300)
            uartServer::gear = '3';
        else if (i < 400)
            uartServer::gear = '4';
        else
            uartServer::gear = '5';

        if ((i % 100)>74)
            uartServer::rpmPercent = 100;
        else 
            uartServer::rpmPercent = 0;
        uartServer::speed = i;
        uartServer::absLevel = i % 100;
        uartServer::engineMap = i % 10;
        uartServer::tcLevel = i % 20;
        delay(100);
    }
    uartServer::gear = ' ';
    delay(3000);
    Serial.println("frame server disabled");
    ui::frameServerSetEnabled(false);
    uartServer::gear = 'E';
    delay(2000);
}