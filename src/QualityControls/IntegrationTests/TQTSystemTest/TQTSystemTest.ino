/**
 * @file TQTSystemTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-02-09
 * @brief System test for the Lilygo T-QT board. See [Readme](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

// #include "debugUtils.h"
#include "SimWheel.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    Serial.println("--READY--");
    userSettings::begin();
    power::begin(GPIO_NUM_0, true);

    inputs::addDigital(GPIO_NUM_0, 0);
    inputs::addDigital(GPIO_NUM_47, 1);

    hidImplementation::begin("TQTSystemTestBLE", "Mamandurrio", true);
    inputs::start();
    power::startBatteryMonitor((gpio_num_t)-1, GPIO_NUM_4, true);
    Serial.println("--GO--");
}

void loop()
{
    // vTaskDelay(portMAX_DELAY);
    Serial.println("--ALIVE--");
    delay(5000);
}