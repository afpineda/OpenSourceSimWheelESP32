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
#include "debugUtils.h"

// ----------------------------------------------------------------------------
// GPIO
// ----------------------------------------------------------------------------

// [EN] Put your GPIO pin numbers here
// [ES] Ponga aquí sus números de pin GPIO
#define BATT_EN_PIN GPIO_NUM_12
#define BATT_READ_PIN GPIO_NUM_2
#define POWER_WITNESS_PIN GPIO_NUM_22

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

#define SAMPLING_MILLIS (60 * 1000) // 1 minute
#define PROGRESS_LOOPS 10

uint64_t loopCount = 0;

// ----------------------------------------------------------------------------
// Auxiliary
// ----------------------------------------------------------------------------

extern void configureBatteryMonitor(
    gpio_num_t enableBatteryReadPin,
    gpio_num_t batteryLevelPin);

void waitForPowerWitness()
{
    bool notReady;
    do
    {
        int v = gpio_get_level(POWER_WITNESS_PIN);
        notReady = (v == 0);
        delay(500);
    } while (notReady);
}

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


// ----------------------------------------------------------------------------
// Arduino entry point
// ----------------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;
    batteryCalibration::clear();
    configureBatteryMonitor(BATT_EN_PIN, BATT_READ_PIN);

    //    pinMode(POWER_WITNESS_PIN, INPUT);
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << POWER_WITNESS_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    loopCount = 0;
    Serial.println("[EN] Ensure battery is fully charged before proceeding.");
    Serial.println("[EN] To continue, wire '3V3' (from the powerboost module) to POWER_WITNESS pin.");
    Serial.println("[ES] Asegurese de que la bateria esta cargada antes de continuar.");
    Serial.println("[ES] Para continuar, conecte '3V3' (del modulo powerboost) al pin POWER_WITNESS.");
    waitForPowerWitness();
    Serial.println("[EN] Calibration in progress.");
    Serial.println("[EN] This may take hours or days. Be patient.");
    serialPrintf("[EN] Progress will be notified every %d minutes more or less.\n",PROGRESS_LOOPS);
    Serial.println("[ES] Calibracion en curso.");
    Serial.println("[ES] Puede tomar horas o dias. Sea paciente.");
    serialPrintf("[ES] El progreso se notificara cada %d minutos mas o menos.\n",PROGRESS_LOOPS);
    Serial.println("");
}

void loop()
{
    int v = gpio_get_level(POWER_WITNESS_PIN);
    if (v == LOW)
    {
        // We are finish !!!
        batteryCalibration::save();
        Serial.println("[EN] Battery is depleted. Calibration data saved to flash memory.");
        Serial.println("[EN] Take note of calibration data for backup.");
        Serial.println("[ES] Bateria agotada. Datos de calibracion salvados en memoria flash.");
        Serial.println("[ES] Tome nota de los datos de calibracion como respaldo.");
        Serial.println("------------");
        dumpCalibrationData();
        Serial.println("--END--FIN--");
        for (;;)
            delay(SAMPLING_MILLIS);
    }
    else
    {
        int reading = power::getBatteryReadingForTesting(BATT_EN_PIN, BATT_READ_PIN);
        batteryCalibration::addSample(reading);
        if ((loopCount % PROGRESS_LOOPS) == 0)
        {
            serialPrintf("[EN] Last reading: %d | Samples: %u\n", reading, loopCount);
            serialPrintf("[ES] Ultima lectura: %d | Muestras: %u\n", reading, loopCount);
        }
    }

    loopCount++;
    delay(SAMPLING_MILLIS);
}