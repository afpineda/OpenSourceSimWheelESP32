/**
 * @file BatteryCalibration.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-08
 * @brief Data acquisition for battery calibration. See [README](./README_en.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include "HAL.hpp"

#include <HardwareSerial.h>

// ----------------------------------------------------------------------------
// GPIO
// ----------------------------------------------------------------------------

// [EN] Put your GPIO pin numbers here
// [ES] Ponga aquí sus números de pin GPIO

#define BATT_EN_PIN GPIO_NUM_42
#define BATT_READ_PIN GPIO_NUM_2

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

#define SAMPLING_MILLIS (60 * 1000) // 1 minute
ADC_GPIO batteryREADPin = BATT_READ_PIN;
OutputGPIO battENPin = BATT_EN_PIN;

// ----------------------------------------------------------------------------
// Auxiliary
// ----------------------------------------------------------------------------

void dumpCalibrationData()
{
    int data;
    Serial.print("{ ");
    for (
        uint8_t index = 0;
        index < BatteryCalibrationService::call::getCalibrationDataCount();
        index++)
    {
        data = BatteryCalibrationService::call::getCalibrationData(index);
        if (index > 0)
            Serial.print(", ");
        Serial.print(data);
    }
    Serial.println(" };");
}

int getBatteryReading()
{
    if (battENPin != UNSPECIFIED::VALUE)
    {
        GPIO_SET_LEVEL(battENPin, 1);
        DELAY_TICKS(200);
    }
    int reading = internals::hal::gpio::getADCreading(batteryREADPin, 100);
    if (battENPin != UNSPECIFIED::VALUE)
    {
        GPIO_SET_LEVEL(battENPin, 0);
    }
    return reading;
}

bool isBatteryPresent()
{
    return (getBatteryReading() < 150);
}

bool isBatteryAlreadyCalibrated()
{
    // If getBatteryLevel() returns -1, there is no calibration data stored
    // in flash memory
    return (BatteryCalibrationService::call::getBatteryLevel(4090) >= 0);
}

void erasePreviousCalibrationData()
{
    Serial.println("===================================");
    Serial.println("CALIBRATION PROCEDURE NOT STARTING");
    Serial.println("===================================");
    Serial.println("You have to EXPLICITLY erase the previous calibration data.");
    Serial.println("TO ERASE it, type any character in the serial monitor and hit ENTER.");
    while (!Serial.available())
    {
        Serial.println("Waiting for user input...");
        DELAY_MS(2000);
    }
    Serial.println("Erasing calibration data...");

    uint8_t calCount = BatteryCalibrationService::call::getCalibrationDataCount();
    for (uint8_t i = 0; i < calCount; i++)
        BatteryCalibrationService::call::setCalibrationData(i, 0, false);
    SaveSetting::notify(UserSetting::BATTERY_CALIBRATION_DATA);
    DELAY_MS(200);

    Serial.println("===================================");
    Serial.println("NEXT STEPS");
    Serial.println("===================================");
    Serial.println("1. Ensure your battery is fully charged.");
    Serial.println("2. Ensure the battery is wired to the battery monitor circuit.");
    Serial.println("3. Remove the USB cable or any wired power supply.");
    Serial.println("4. Power the DevKit using the battery alone.");
    Serial.println("5. Open the Bluetooth control panel and pair the device (if not done yet).");
    Serial.println("The battery calibration procedure will then start immediately.");
    Serial.println("Just wait for the battery to deplete.");
    Serial.println("No human supervision is required.");
    Serial.println("===================================");
    while (!isBatteryPresent())
    {
        Serial.println("Note: Battery not detected...");
        DELAY_MS(10000);
    }
    Serial.println("Note: Battery detected (you are good to go)...");
    Serial.println("Now stopped until reset or power removal.");
    for (;;)
        DELAY_MS(10000);
}

// ----------------------------------------------------------------------------
// Mocks
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Arduino entry point
// ----------------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    try
    {
        Serial.println("===================================");
        Serial.println("=  Battery calibration procedure  =");
        Serial.println("===================================");

        // Inform about battery detection
        Serial.print("Battery presence: ");
        if (isBatteryPresent())
            Serial.println("yes");
        else
            Serial.println("no");

        // Initialize required firmware subsystems
        internals::storage::getReady();
        internals::batteryCalibration::getReady();
        OnStart::notify(); // Stored calibration data is loaded from flash memory here

        // Look for previously stored calibration data
        Serial.print("Battery calibration data already stored: ");
        if (isBatteryAlreadyCalibrated())
        {
            Serial.println("yes");
            Serial.println("===================================");
            Serial.println("Current calibration data follows. Write down for backup:");
            Serial.println("");
            dumpCalibrationData();
            Serial.println("");
            erasePreviousCalibrationData(); // Does not return
        }
        else
        {
            Serial.println("no");
            // Wait for battery presence
            while (!isBatteryPresent())
            {
                Serial.println("Battery not present. Waiting...");
                DELAY_MS(5000);
            }

            hid::configure("Battery calibration", "Mamandurrio", false);
            internals::hid::common::getReady();
            OnStart::notify();
            internals::batteryCalibration::clear();

            // Wait for BLE connection
            while (!internals::hid::isConnected())
            {
                Serial.println("Waiting for Bluetooth connection...");
                DELAY_MS(5000);
            }

            Serial.println("===================================");
            Serial.println("The battery calibration procedure has started.");
        }
    }
    catch (std::exception &e)
    {
        // Delay to allow log_e() to print text
        // (may be running in another thread)
        vTaskDelay(pdMS_TO_TICKS(500));

        for (;;)
        {
            Serial.println("**FIRMWARE ERROR**");
            Serial.println(e.what());
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }
}

void loop()
{
    int reading = getBatteryReading();
    if (reading >= 150)
    {
        internals::batteryCalibration::addSample(reading);
        SaveSetting::notify(UserSetting::BATTERY_CALIBRATION_DATA);
        // The user does not see this message,
        // but can see the light in the TX LED (if available)
        Serial.println("running");
    } // else BATT_READ_PIN is not connected
    DELAY_MS(SAMPLING_MILLIS);
}