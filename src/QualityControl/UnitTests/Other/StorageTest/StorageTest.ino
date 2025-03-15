/**
 * @file StorageTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-01-13
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalTypes.hpp"
#include "ServiceMocks.hpp"

#include "HardwareSerial.h"
#include "nvs_flash.h"

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

//------------------------------------------------------------------

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

void saved()
{
    Serial.println("Saved!");
}

void setTestCaseForDelayedSettings()
{
    InputService::call::setAxisCalibration(0, 1, 2, 3, false);
    InputService::call::setAxisPolarity(false, true, false);
    InputService::call::setRotaryPulseWidthMultiplier(PulseWidthMultiplier::X6, false);
    InputHubService::call::setAltButtonsWorkingMode(AltButtonsWorkingMode::Regular, false);
    InputHubService::call::setBitePoint(12, false);
    InputHubService::call::setClutchWorkingMode(ClutchWorkingMode::LAUNCH_CONTROL_MASTER_LEFT, false);
    InputHubService::call::setDPadWorkingMode(DPadWorkingMode::Regular, false);
    InputHubService::call::setSecurityLock(true, false);
}

void checkTestCaseForDelayedSettings()
{
    int a, b, c, d;
    Serial.println("Checking current settings against test case (delayed settings)");
    InputService::call::getAxisCalibration(a, b, c, d);
    if ((a != 0) || (b != 1) || (c != 2) || (d != 3))
        Serial.printf("NO MATCH: axis calibration data (%d,%d,%d,%d)\n",
                      a, b, c, d);
    bool x, y;
    InputService::call::getAxisPolarity(x, y);
    if (x || !y)
        Serial.println("NO MATCH: axis polarity");
    auto pw = InputService::call::getRotaryPulseWidthMultiplier();
    if (pw != PulseWidthMultiplier::X6)
        Serial.println("NO MATCH: pulse width");
    auto alt_WM = InputHubService::call::getAltButtonsWorkingMode();
    if (alt_WM != AltButtonsWorkingMode::Regular)
        Serial.println("NO MATCH: Alt buttons working mode");
    auto bitePoint = InputHubService::call::getBitePoint();
    if (bitePoint != 12)
        Serial.printf("NO MATCH: Bite point (%hhu)\n", bitePoint);
    auto clutch_WM = InputHubService::call::getClutchWorkingMode();
    if (clutch_WM != ClutchWorkingMode::LAUNCH_CONTROL_MASTER_LEFT)
        Serial.println("NO MATCH: Clutch working mode");
    auto dpad_WM = InputHubService::call::getDPadWorkingMode();
    if (dpad_WM != DPadWorkingMode::Regular)
        Serial.println("NO MATCH: DPAD working mode");
    x = InputHubService::call::getSecurityLock();
    if (!x)
        Serial.println("NO MATCH: security lock");
    Serial.println("Done");
}

void setTestCaseForNonDelayedSettings()
{
    InputMapService::call::setMap(64, 126, 125);
    HidService::call::setCustomHardwareID(0xFEFE, 0xEFEF, false);
    BatteryCalibrationService::call::setAutoCalibrationParameter(3999, false);
    BatteryCalibrationService::call::setCalibrationData(0, 101, false);
}

void checkTestCaseForNonDelayedSettings()
{
    uint8_t a, b;
    uint16_t x, y;
    InputMapService::call::getMap(43, a, b);
    if ((a != 126) || (b != 125))
        Serial.println("NO MATCH: input map");
    HidService::call::getCustomHardwareID(x, y);
    if ((x != 0xFEFE) || (y != 0xEFEF))
        Serial.println("NO MATCH: hardware ID");
    int cal = BatteryCalibrationService::call::getAutoCalibrationParameter();
    if (cal != 3999)
        Serial.println("NO MATCH: auto-calibration parameter");
    cal = BatteryCalibrationService::call::getCalibrationData(0);
    if (cal != 101)
        Serial.println("NO MATCH: calibration data");
    Serial.println("Done");
}

//------------------------------------------------------------------
// Test groups
//------------------------------------------------------------------

void cleanMrProper()
{
    nvs_flash_erase();
    Serial.println("** The NVS flash partition has been erased **");
    Serial.println("");
}

void TG_save()
{
    setTestCaseForDelayedSettings();
    setTestCaseForNonDelayedSettings();
    SaveSetting::notify(UserSetting::ALL);
}

void TG_saveDelayed()
{
    setTestCaseForDelayedSettings();
    SaveSetting::notify(UserSetting::AXIS_CALIBRATION);
    SaveSetting::notify(UserSetting::AXIS_POLARITY);
    SaveSetting::notify(UserSetting::PULSE_WIDTH);
    SaveSetting::notify(UserSetting::ALT_WORKING_MODE);
    SaveSetting::notify(UserSetting::BITE_POINT);
    SaveSetting::notify(UserSetting::CLUTCH_WORKING_MODE);
    SaveSetting::notify(UserSetting::DPAD_WORKING_MODE);
    SaveSetting::notify(UserSetting::SECURITY_LOCK);
    Serial.println("Save settings requested (there is a 20 seconds delay). Wait.");
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    try
    {
        InputService::inject(new FakeInputService());
        InputHubService::inject(new FakeInputHubService());
        BatteryCalibrationService::inject(new FakeBatteryCalibration());
        HidService::inject(new FakeHidService());
        InputMapService::inject(new FakeInputMapService());

        internals::storage::getReady();
        OnSettingsSaved::subscribe(saved);
        OnStart::notify();
        LoadSetting::notify(UserSetting::ALL);

        Serial.println("All settings should be loaded.");
        Serial.println("-- Select test case --");
        Serial.println("1 = Save test case for delayed settings");
        Serial.println("2 = Save test case (all settings)");
        Serial.println("3 = Check loaded settings (delayed saving)");
        Serial.println("4 = Check loaded settings (not delayed saving)");
        Serial.println("X = ERASE NVS flash partition (CAUTION: will erase everything)");
        Serial.println("----------------------");

        int c = 0;
        while (c == 0)
        {
            while (!Serial.available())
                ;
            c = Serial.peek();
            while (Serial.read() >= 0)
                ;
            if (c == '1')
                TG_saveDelayed();
            else if (c == '2')
                TG_save();
            else if (c == '3')
                checkTestCaseForDelayedSettings();
            else if (c == '4')
                checkTestCaseForNonDelayedSettings();
            else if (c == 'X')
                cleanMrProper();
            else
            {
                c = 0;
                Serial.println("Try again");
            }
        }
        Serial.printf("-- Command %c executed --\n", c);
        Serial.println("Reset to continue or wait for settings to be saved.");
    }
    catch (std::exception &e)
    {
        Serial.println("Exception:");
        Serial.println(e.what());
    }
}

//------------------------------------------------------------------

void loop()
{
}