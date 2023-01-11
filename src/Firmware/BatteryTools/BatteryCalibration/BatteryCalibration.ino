/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-08
 * @brief Data acquisition for battery calibration. See [README](./README_en.md)
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "SimWheel.h"
// #include "debugUtils.h"

// ----------------------------------------------------------------------------
// GPIO
// ----------------------------------------------------------------------------

// [EN] Put your GPIO pin numbers here
// [ES] Ponga aquí sus números de pin GPIO
#define BATT_EN_PIN GPIO_NUM_17
#define BATT_READ_PIN GPIO_NUM_15

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

#define SAMPLING_MILLIS (60 * 1000) // 1 minute

uint64_t loopCount = 0;

// ----------------------------------------------------------------------------
// Auxiliary
// ----------------------------------------------------------------------------

extern void configureBatteryMonitor(
    gpio_num_t enableBatteryReadPin,
    gpio_num_t batteryLevelPin);

void dumpCalibrationData()
{
    Serial.print("{ ");
    uint8_t index = 0;
    int data = batteryCalibration::getCalibration(index);
    while (data >= 0)
    {
        if (index > 0)
            Serial.print(", ");
        Serial.print(data);
        index++;
        data = batteryCalibration::getCalibration(index);
    }
    Serial.println(" };");
}

bool serialBlocked = true;

void testUSB(void *unused)
{
    Serial.begin(115200); // will block if USB not connected
    while (!Serial)
        delay(250);
    Serial.println("--START--");
    Serial.flush();
    serialBlocked = false;
    for (;;)
        delay(1000);
}

// ----------------------------------------------------------------------------
// Mocks
// ----------------------------------------------------------------------------

void inputs::update()
{
}

void inputs::recalibrateAxes()
{
}

// ----------------------------------------------------------------------------
// Arduino entry point
// ----------------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);

    // int countdown = 60;
    int countdown = 10;
    TaskHandle_t testUSBtask = nullptr;
    if ((xTaskCreate(testUSB, "usbTest", 2048, nullptr, tskIDLE_PRIORITY + 1, &testUSBtask) != pdPASS) || (!testUSBtask))
    {
        log_e("Unable to create testUSB task");
        abort();
    };
    delay(500);
    while ((serialBlocked) && (countdown > 0))
    {
        countdown--;
        delay(1000);
    }
    vTaskDelete(testUSBtask);
    
    // Serial.begin(115200);
    // while ((!Serial) && (countdown>0)) {
    //     countdown--;
    //     delay(1000);
    // }

    if (serialBlocked)
//    if (!Serial)
    {
        batteryCalibration::clear();
        // batteryCalibration::save();
        hidImplementation::begin("Battery calibration", "Mamandurrio", false);
    }
    else
    {
        batteryCalibration::begin();
        Serial.println("[EN] Current battery calibration data:");
        Serial.println("[ES] Datos actuales de calibracion de bateria:");
        Serial.println("----------------------------------------------");
        dumpCalibrationData();
        Serial.println("----------------------------------------------");
        Serial.println("--END--FIN--");
        Serial.end();
        for (;;)
            delay(1000);
    }
}

void loop()
{
    delay(SAMPLING_MILLIS);
    int reading = power::getBatteryReadingForTesting(BATT_EN_PIN, BATT_READ_PIN);
    // if (reading >= 150)
    //     batteryCalibration::addSample(reading, true);
    // // else BATT_READ_PIN is not connected
}