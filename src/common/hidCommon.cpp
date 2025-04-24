
/**
 * @file hidCommon.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-01-12
 *
 * @brief Common functionality to all HID implementations
 *
 * @copyright Licensed under the EUPL
 *
 */

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include "HID_definitions.hpp"

#if !CD_CI
#include "esp_mac.h"
#endif

#include <string>
// #include <iostream> // For testing

//-------------------------------------------------------------------
// Globals
//-------------------------------------------------------------------

uint8_t selectedInput = 0xFF;
uint8_t selected_ui = 0xFF;

//-------------------------------------------------------------------

uint16_t _factoryVID = BLE_VENDOR_ID;
uint16_t _factoryPID = BLE_PRODUCT_ID;
#define MAX_INPUT_NUMBER 63

std::string _deviceName = "ESP32SimWheel";
std::string _deviceManufacturer = "Mamandurrio";
bool _autoPowerOff = true;

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Public namespace
//-------------------------------------------------------------------
//-------------------------------------------------------------------

void hid::configure(
    std::string deviceName,
    std::string deviceManufacturer,
    bool enableAutoPowerOff,
    uint16_t vendorID,
    uint16_t productID)
{
    if (vendorID == 0)
        vendorID = BLE_VENDOR_ID;
    if (productID == 0)
        productID = BLE_PRODUCT_ID;

    _deviceName = deviceName;
    _deviceManufacturer = deviceManufacturer;
    _autoPowerOff = enableAutoPowerOff;
    _factoryVID = vendorID;
    _factoryPID = productID;
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Internal namespace
//-------------------------------------------------------------------
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// Service class
//-------------------------------------------------------------------

class HidServiceProvider : public HidService
{
public:
    virtual void getCustomHardwareID(uint16_t &customVID, uint16_t &customPID) override
    {
        customVID = _customVID;
        customPID = _customVID;
    }

    virtual void setCustomHardwareID(uint16_t customVID, uint16_t customPID, bool save) override
    {
        _customVID = customVID;
        _customPID = customPID;
        if (save)
            SaveSetting::notify(UserSetting::CUSTOM_HARDWARE_ID);
    }

    virtual void setFactoryDefaultHardwareID()
    {
        _customVID = 0;
        _customPID = 0;
        SaveSetting::notify(UserSetting::CUSTOM_HARDWARE_ID);
    }

private:
    inline static uint16_t _customVID = 0;
    inline static uint16_t _customPID = 0;
};

//-------------------------------------------------------------------
// Start
//-------------------------------------------------------------------

void commonHidStart()
{
    uint16_t customVID = 0;
    uint16_t customPID = 0;
    if (internals::hid::supportsCustomHardwareID())
    {
        if ((_factoryVID == TEST_HARDWARE_ID) && (_factoryPID == TEST_HARDWARE_ID))
        {
            // For testing, do not load the custom hardware ID
            customVID = BLE_PRODUCT_ID;
            customPID = BLE_PRODUCT_ID;
        }
        else
        {
            // Load the custom hardware ID
            LoadSetting::notify(UserSetting::CUSTOM_HARDWARE_ID);
            HidService::call::getCustomHardwareID(customVID, customPID);
            if ((customVID == 0) && (customPID == 0))
            {
                // There is no custom hardware ID, use factory defaults
                customVID = _factoryVID;
                customPID = _factoryPID;
            }
        }
    }
    internals::hid::begin(
        _deviceName,
        _deviceManufacturer,
        _autoPowerOff,
        customVID,
        customPID);
}

//-------------------------------------------------------------------

void internals::hid::common::getReady()
{
    OnStart::subscribe(commonHidStart);
    OnBatteryLevel::subscribe(internals::hid::reportBatteryLevel);
    HidService::inject(new HidServiceProvider());
}

//-------------------------------------------------------------------
// Feature reports
//-------------------------------------------------------------------

uint16_t internals::hid::common::onGetFeature(
    uint8_t report_id,
    uint8_t *buffer,
    uint16_t len)
{
    if ((report_id == RID_FEATURE_CAPABILITIES) && (len >= CAPABILITIES_REPORT_SIZE))
    {
        buffer[0] = MAGIC_NUMBER_LOW;
        buffer[1] = MAGIC_NUMBER_HIGH;
        *(uint16_t *)(buffer + 2) = DATA_MAJOR_VERSION;
        *(uint16_t *)(buffer + 4) = DATA_MINOR_VERSION;
        *(uint16_t *)(buffer + 6) = DeviceCapabilities::getFlags();
        *(uint64_t *)(buffer + 8) = 0ULL;
#if !CD_CI
        esp_efuse_mac_get_default(buffer + 8);
#endif
        buffer[16] = UIService::call::getMaxFPS();
        buffer[17] = internals::pixels::getCount(PixelGroup::GRP_TELEMETRY);
        buffer[18] = internals::pixels::getCount(PixelGroup::GRP_BUTTONS);
        buffer[19] = internals::pixels::getCount(PixelGroup::GRP_INDIVIDUAL);
        return CAPABILITIES_REPORT_SIZE;
    }
    if ((report_id == RID_FEATURE_CONFIG) && (len >= CONFIG_REPORT_SIZE))
    {
        buffer[0] = (uint8_t)InputHubService::call::getClutchWorkingMode();
        buffer[1] = (uint8_t)InputHubService::call::getAltButtonsWorkingMode();
        buffer[2] = (uint8_t)InputHubService::call::getBitePoint();
        buffer[3] = (uint8_t)BatteryService::call::getLastBatteryLevel();
        buffer[4] = (uint8_t)InputHubService::call::getDPadWorkingMode();
        buffer[5] = (InputHubService::call::getSecurityLock()) ? 0xFF : 0x00;
        buffer[6] = (uint8_t)InputService::call::getRotaryPulseWidthMultiplier();
        return CONFIG_REPORT_SIZE;
    }
    if ((report_id == RID_FEATURE_BUTTONS_MAP) && (len >= BUTTONS_MAP_REPORT_SIZE))
    {
        buffer[0] = selectedInput;
        if ((selectedInput <= MAX_INPUT_NUMBER) && InputNumber::booked(selectedInput))
        {
            InputMapService::call::getMap(selectedInput, buffer[1], buffer[2]);
        }
        else
        {
            buffer[1] = 0xFF;
            buffer[2] = 0xFF;
        }
        return BUTTONS_MAP_REPORT_SIZE;
    }
    if ((report_id == RID_FEATURE_HARDWARE_ID) && (len >= HARDWARE_ID_REPORT_SIZE))
    {
        buffer[4] = 0;
        buffer[5] = 0;
        if (hid::supportsCustomHardwareID())
        {
            // BLE
            uint16_t vid, pid;
            HidService::call::getCustomHardwareID(vid, pid);
            if ((vid == 0) && (pid == 0))
            {
                vid = _factoryVID;
                pid = _factoryPID;
            }
            *(uint16_t *)(buffer) = vid;
            *(uint16_t *)(buffer + 2) = pid;
        }
        else
        {
            // USB
            buffer[0] = 0;
            buffer[1] = 0;
            buffer[2] = 0;
            buffer[3] = 0;
        }
        return HARDWARE_ID_REPORT_SIZE;
    }
    return 0;
}

//-------------------------------------------------------------------

void internals::hid::common::onSetFeature(
    uint8_t report_id,
    const uint8_t *buffer,
    uint16_t len)
{
    if ((InputHubService::call::getSecurityLock()) || (report_id == RID_FEATURE_CAPABILITIES))
        return;
    if (report_id == RID_FEATURE_CONFIG)
    {
        if ((len > 0) && (buffer[0] >= (uint8_t)ClutchWorkingMode::CLUTCH) && (buffer[0] <= (uint8_t)ClutchWorkingMode::_MAX_VALUE))
        {
            // clutch working mode
            InputHubService::call::setClutchWorkingMode((ClutchWorkingMode)buffer[0]);
        }
        if ((len > 1) && (buffer[1] != 0xff))
        {
            // ALT Buttons working mode
            AltButtonsWorkingMode cwm = (buffer[1] == 0) ? AltButtonsWorkingMode::Regular : AltButtonsWorkingMode::ALT;
            InputHubService::call::setAltButtonsWorkingMode(cwm);
        }
        if ((len > 2) && (buffer[2] <= CLUTCH_FULL_VALUE)) // && ((uint8_t)buffer[2] >= CLUTCH_NONE_VALUE)
        {
            // Bite point
            InputHubService::call::setBitePoint(buffer[2]);
        }
        if ((len > 3) && (buffer[3] == (uint8_t)SimpleCommand::CMD_AXIS_RECALIBRATE))
        {
            // Force analog axis recalibration
            InputService::call::recalibrateAxes();
        }
        if ((len > 3) && (buffer[3] == (uint8_t)SimpleCommand::CMD_BATT_RECALIBRATE))
        {
            // Restart auto calibration algorithm
            BatteryCalibrationService::call::restartAutoCalibration();
        }
        if ((len > 3) && (buffer[3] == (uint8_t)SimpleCommand::CMD_RESET_BUTTONS_MAP))
        {
            // Reset buttons map to factory defaults
            InputMapService::call::resetMap();
        }
        if ((len > 3) && (buffer[3] == (uint8_t)SimpleCommand::CMD_SAVE_NOW))
        {
            // save settings now
            SaveSetting::notify(UserSetting::ALL);
        }
        if ((len > 3) && (buffer[3] == (uint8_t)SimpleCommand::CMD_REVERSE_LEFT_AXIS))
        {
            // change left axis polarity
            InputService::call::reverseLeftAxis();
        }
        if ((len > 3) && (buffer[3] == (uint8_t)SimpleCommand::CMD_REVERSE_RIGHT_AXIS))
        {
            // change left axis polarity
            InputService::call::reverseRightAxis();
        }
        if ((len > 3) && (buffer[3] == (uint8_t)SimpleCommand::CMD_SHOW_PIXELS))
        {
            // Show all pixels at once
            internals::pixels::show();
        }
        if ((len > 3) && (buffer[3] == (uint8_t)SimpleCommand::CMD_RESET_PIXELS))
        {
            // Turn off all pixels
            internals::pixels::reset();
        }
        if ((len > 4) && (buffer[4] != 0xff))
        {
            // Set working mode of DPAD
            DPadWorkingMode mode = (buffer[4] == 0) ? DPadWorkingMode::Regular : DPadWorkingMode::Navigation;
            InputHubService::call::setDPadWorkingMode(mode);
        }
        // Note: byte index 5 is read-only
        if ((len > 6) && (buffer[6] != 0xff))
        {
            // Set a pulse width multiplier for all rotary encoders
            if ((buffer[6] >= (uint8_t)PulseWidthMultiplier::X1) &&
                (buffer[6] <= (uint8_t)PulseWidthMultiplier::_MAX_VALUE))
            {
                PulseWidthMultiplier mult = static_cast<PulseWidthMultiplier>(buffer[6]);
                InputService::call::setRotaryPulseWidthMultiplier(mult);
            }
        }
    }
    else if ((report_id == RID_FEATURE_BUTTONS_MAP) && (len >= BUTTONS_MAP_REPORT_SIZE))
    {
        if (buffer[0] <= MAX_INPUT_NUMBER)
        {
            selectedInput = buffer[0];
            if ((buffer[1] <= MAX_INPUT_NUMBER) && (buffer[2] <= MAX_INPUT_NUMBER))
                InputMapService::call::setMap(buffer[0], buffer[1], buffer[2]);
        }
    }
    else if ((report_id == RID_FEATURE_HARDWARE_ID) && (len >= HARDWARE_ID_REPORT_SIZE))
    {
        if (hid::supportsCustomHardwareID())
        {
            uint16_t vid = *(uint16_t *)(buffer);
            uint16_t pid = *(uint16_t *)(buffer + 2);
            uint16_t control_code = *(uint16_t *)(buffer + 4);
            uint16_t expected_code;

            if ((vid == 0) || (pid == 0))
                expected_code = 0xAA96;
            else
                expected_code = (vid * pid) % 65536;

            if (control_code == expected_code)
                HidService::call::setCustomHardwareID(vid, pid);
        } // else ignore
    }
    else
        assert("Set feature report: Unknown ID");
}

//-------------------------------------------------------------------
// Output reports
//-------------------------------------------------------------------

void internals::hid::common::onOutput(
    uint8_t report_id,
    const uint8_t *buffer,
    uint16_t len)
{
    // TODO: security lock
    if ((report_id == RID_OUTPUT_POWERTRAIN) && (len >= POWERTRAIN_REPORT_SIZE))
    {
        telemetry::data.powertrain.gear = (char)buffer[0];
        telemetry::data.powertrain.rpm = *((uint16_t *)(buffer + 1));
        telemetry::data.powertrain.rpmPercent = buffer[3];
        if (telemetry::data.powertrain.rpmPercent > 100)
            telemetry::data.powertrain.rpmPercent = 100;
        telemetry::data.powertrain.shiftLight1 = buffer[4];
        telemetry::data.powertrain.shiftLight2 = buffer[5];
        telemetry::data.powertrain.revLimiter = buffer[6];
        telemetry::data.powertrain.engineStarted = buffer[7];
        telemetry::data.powertrain.speed = *((uint16_t *)(buffer + 8));
    }
    else if ((report_id == RID_OUTPUT_ECU) && (len >= ECU_REPORT_SIZE))
    {
        telemetry::data.ecu.absEngaged = buffer[0];
        telemetry::data.ecu.tcEngaged = buffer[1];
        telemetry::data.ecu.drsEngaged = buffer[2];
        telemetry::data.ecu.pitLimiter = buffer[3];
        telemetry::data.ecu.lowFuelAlert = buffer[4];
        telemetry::data.ecu.absLevel = buffer[5];
        telemetry::data.ecu.tcLevel = buffer[6];
        telemetry::data.ecu.tcCut = buffer[7];
        telemetry::data.ecu.brakeBias = buffer[8];
        if (telemetry::data.ecu.brakeBias > 100)
            telemetry::data.ecu.brakeBias = 100;
    }
    else if ((report_id == RID_OUTPUT_RACE_CONTROL) && (len >= RACE_CONTROL_REPORT_SIZE))
    {
        telemetry::data.raceControl.blackFlag = buffer[0];
        telemetry::data.raceControl.blueFlag = buffer[1];
        telemetry::data.raceControl.checkeredFlag = buffer[2];
        telemetry::data.raceControl.greenFlag = buffer[3];
        telemetry::data.raceControl.orangeFlag = buffer[4];
        telemetry::data.raceControl.whiteFlag = buffer[5];
        telemetry::data.raceControl.yellowFlag = buffer[6];
        telemetry::data.raceControl.remainingLaps = *((uint16_t *)(buffer + 7));
        telemetry::data.raceControl.remainingMinutes = *((uint16_t *)(buffer + 9));
    }
    else if ((report_id == RID_OUTPUT_GAUGES) && (len >= GAUGES_REPORT_SIZE))
    {
        telemetry::data.gauges.relativeTurboPressure = buffer[0];
        if (telemetry::data.gauges.relativeTurboPressure > 100)
            telemetry::data.gauges.relativeTurboPressure = 100;
        telemetry::data.gauges.absoluteTurboPressure = static_cast<float>(*((uint16_t *)(buffer + 1)) / 100.0);
        telemetry::data.gauges.waterTemperature = *((uint16_t *)(buffer + 3));
        telemetry::data.gauges.oilPressure = static_cast<float>(*((uint16_t *)(buffer + 5)) / 100.0);
        telemetry::data.gauges.oilTemperature = *((uint16_t *)(buffer + 7));
        telemetry::data.gauges.relativeRemainingFuel = buffer[9];
        if (telemetry::data.gauges.relativeRemainingFuel > 100)
            telemetry::data.gauges.relativeRemainingFuel = 100;
        telemetry::data.gauges.absoluteRemainingFuel = *((uint16_t *)(buffer + 10));
    }
    else if ((report_id == RID_OUTPUT_PIXEL) && (len >= PIXEL_REPORT_SIZE))
    {
        if (buffer[0] == 0xFF)
        {
            internals::pixels::show();
            return;
        }
        if (buffer[0] == 0xFE)
        {
            internals::pixels::reset();
            return;
        }
        if (buffer[0] > (uint8_t)PixelGroup::GRP_INDIVIDUAL)
            // Invalid pixel group
            return;

        internals::pixels::set(
            (PixelGroup)buffer[0],
            buffer[1],
            buffer[4],
            buffer[3],
            buffer[2]);
        return;
    }
    telemetry::data.frameID = telemetry::data.frameID + 1;
    // log_d("frame id: %u", telemetry::data.frameID);
}

//-------------------------------------------------------------------
// Input reports
//-------------------------------------------------------------------

void internals::hid::common::onReset(uint8_t *report)
{
    report[0] = 0;
    report[1] = 0;
    report[2] = 0;
    report[3] = 0;
    report[4] = 0;
    report[5] = 0;
    report[6] = 0;
    report[7] = 0;
    report[8] = 0;
    report[9] = 0;
    report[10] = 0;
    report[11] = 0;
    report[12] = 0;
    report[13] = 0;
    report[14] = 0;
    report[15] = 0;
    report[16] = CLUTCH_NONE_VALUE;
    report[17] = CLUTCH_NONE_VALUE;
    report[18] = CLUTCH_NONE_VALUE;
    report[19] = 0;
}

//-------------------------------------------------------------------

void internals::hid::common::onReportInput(
    uint8_t *report,
    bool &notifyConfigChanges,
    uint64_t &inputsLow,
    uint64_t &inputsHigh,
    uint8_t &POVstate,
    uint8_t &leftAxis,
    uint8_t &rightAxis,
    uint8_t &clutchAxis)
{
    report[0] = ((uint8_t *)&inputsLow)[0];
    report[1] = ((uint8_t *)&inputsLow)[1];
    report[2] = ((uint8_t *)&inputsLow)[2];
    report[3] = ((uint8_t *)&inputsLow)[3];
    report[4] = ((uint8_t *)&inputsLow)[4];
    report[5] = ((uint8_t *)&inputsLow)[5];
    report[6] = ((uint8_t *)&inputsLow)[6];
    report[7] = ((uint8_t *)&inputsLow)[7];
    report[8] = ((uint8_t *)&inputsHigh)[0];
    report[9] = ((uint8_t *)&inputsHigh)[1];
    report[10] = ((uint8_t *)&inputsHigh)[2];
    report[11] = ((uint8_t *)&inputsHigh)[3];
    report[12] = ((uint8_t *)&inputsHigh)[4];
    report[13] = ((uint8_t *)&inputsHigh)[5];
    report[14] = ((uint8_t *)&inputsHigh)[6];
    report[15] = ((uint8_t *)&inputsHigh)[7];
    report[16] = (uint8_t)clutchAxis;
    report[17] = (uint8_t)leftAxis;
    report[18] = (uint8_t)rightAxis;
    report[19] = POVstate;
    if (notifyConfigChanges)
    {
        report[19] |= (RID_FEATURE_CONFIG << 4);
        notifyConfigChanges = false;
    }
}
