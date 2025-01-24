/**
 * @file hidImplementation_common.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-01-12
 *
 * @brief Common functionality to all HID implementations
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.h"
#include "SimWheelTypes.h"
#include "HID_definitions.h"
#include "esp_mac.h"
#include <Preferences.h>

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

inputNumber_t selectedInput = UNSPECIFIED_INPUT_NUMBER;
uint8_t selected_ui = 0xFF;
#define UNDEFINED_ID 0
uint16_t factoryVID = UNDEFINED_ID;
uint16_t factoryPID = UNDEFINED_ID;
uint16_t customVID = UNDEFINED_ID;
uint16_t customPID = UNDEFINED_ID;

#define PREFS_NAMESPACE "hid"
#define KEY_VID "vid"
#define KEY_PID "pid"

// ----------------------------------------------------------------------------
// Custom hardware ID
// ----------------------------------------------------------------------------

void hidImplementation::common::setFactoryHardwareID(uint16_t vid, uint16_t pid)
{
    factoryVID = vid;
    factoryPID = pid;
    customVID = vid;
    customPID = pid;
}

void hidImplementation::common::loadHardwareID(uint16_t &vid, uint16_t &pid)
{
    Preferences prefs;
    if (prefs.begin(PREFS_NAMESPACE, true))
    {
        customVID = prefs.getUShort(KEY_VID, factoryVID);
        customPID = prefs.getUShort(KEY_PID, factoryPID);
        prefs.end();
    }
    vid = customVID;
    pid = customPID;
}

void hidImplementation::common::storeHardwareID(uint16_t vid, uint16_t pid)
{
    Preferences prefs;
    if (prefs.begin(PREFS_NAMESPACE, false))
    {
        customVID = vid;
        customPID = pid;
        prefs.putUShort(KEY_VID, vid);
        prefs.putUShort(KEY_PID, pid);
        prefs.end();
    }
}

void hidImplementation::common::clearStoredHardwareID()
{
    Preferences prefs;
    if (prefs.begin(PREFS_NAMESPACE, false))
    {
        customVID = factoryVID;
        customPID = factoryPID;
        prefs.remove(KEY_VID);
        prefs.remove(KEY_PID);
        prefs.end();
    }
}

// ----------------------------------------------------------------------------
// Feature reports
// ----------------------------------------------------------------------------

uint16_t hidImplementation::common::onGetFeature(uint8_t report_id, uint8_t *buffer, uint16_t len)
{
    if ((report_id == RID_FEATURE_CAPABILITIES) && (len >= CAPABILITIES_REPORT_SIZE))
    {
        buffer[0] = MAGIC_NUMBER_LOW;
        buffer[1] = MAGIC_NUMBER_HIGH;
        *(uint16_t *)(buffer + 2) = DATA_MAJOR_VERSION;
        *(uint16_t *)(buffer + 4) = DATA_MINOR_VERSION;
        *(uint16_t *)(buffer + 6) = capabilities::flags;
        *(uint64_t *)(buffer + 8) = 0ULL;
        esp_efuse_mac_get_default(buffer + 8);
        buffer[16] = notify::maxFPS;
        buffer[17] = pixels::getPixelCount(pixelGroup_t::GRP_TELEMETRY);
        buffer[18] = pixels::getPixelCount(pixelGroup_t::GRP_BUTTONS);
        buffer[19] = pixels::getPixelCount(pixelGroup_t::GRP_INDIVIDUAL);
        return CAPABILITIES_REPORT_SIZE;
    }
    if ((report_id == RID_FEATURE_CONFIG) && (len >= CONFIG_REPORT_SIZE))
    {
        buffer[0] = (uint8_t)userSettings::cpWorkingMode;
        buffer[1] = (uint8_t)userSettings::altButtonsWorkingMode;
        buffer[2] = (uint8_t)userSettings::bitePoint;
        buffer[3] = (uint8_t)batteryMonitor::getLastBatteryLevel();
        buffer[4] = (uint8_t)userSettings::dpadWorkingMode;
        buffer[5] = (userSettings::securityLock) ? 0xFF : 0x00;
        return CONFIG_REPORT_SIZE;
    }
    if ((report_id == RID_FEATURE_BUTTONS_MAP) && (len >= BUTTONS_MAP_REPORT_SIZE))
    {
        buffer[0] = selectedInput;
        if ((selectedInput <= MAX_INPUT_NUMBER) && (capabilities::availableInputs & BITMAP(selectedInput)))
        {
            userSettings::getEffectiveButtonMap(selectedInput, buffer[1], buffer[2]);
        }
        else
        {
            buffer[1] = UNSPECIFIED_INPUT_NUMBER;
            buffer[2] = UNSPECIFIED_INPUT_NUMBER;
        }
        return BUTTONS_MAP_REPORT_SIZE;
    }
    if ((report_id == RID_FEATURE_HARDWARE_ID) && (len >= HARDWARE_ID_REPORT_SIZE))
    {
        *(uint16_t *)(buffer) = customVID;
        *(uint16_t *)(buffer + 2) = customPID;
        *(uint16_t *)(buffer + 4) = 0;
        return HARDWARE_ID_REPORT_SIZE;
    }
    return 0;
}

// ----------------------------------------------------------------------------

void hidImplementation::common::onSetFeature(uint8_t report_id, const uint8_t *buffer, uint16_t len)
{
    if ((userSettings::securityLock) || (report_id == RID_FEATURE_CAPABILITIES))
        return;
    if (report_id == RID_FEATURE_CONFIG)
    {
        if ((len > 0) && (buffer[0] >= CF_CLUTCH) && (buffer[0] <= CF_LAUNCH_CONTROL_MASTER_RIGHT))
        {
            // clutch function
            userSettings::setCPWorkingMode((clutchFunction_t)buffer[0]);
        }
        if ((len > 1) && (buffer[1] != 0xff))
        {
            // ALT Buttons mode
            userSettings::setALTButtonsWorkingMode((bool)buffer[1]);
        }
        if ((len > 2) && ((clutchValue_t)buffer[2] <= CLUTCH_FULL_VALUE)) // && ((clutchValue_t)buffer[2] >= CLUTCH_NONE_VALUE)
        {
            // Bite point
            userSettings::setBitePoint((clutchValue_t)buffer[2]);
        }
        if ((len > 3) && (buffer[3] == (uint8_t)simpleCommands_t::CMD_AXIS_RECALIBRATE))
        {
            // Force analog axis recalibration
            inputs::recalibrateAxes();
        }
        if ((len > 3) && (buffer[3] == (uint8_t)simpleCommands_t::CMD_BATT_RECALIBRATE))
        {
            // Restart auto calibration algorithm
            batteryCalibration::restartAutoCalibration();
        }
        if ((len > 3) && (buffer[3] == (uint8_t)simpleCommands_t::CMD_RESET_BUTTONS_MAP))
        {
            // Reset buttons map to factory defaults
            userSettings::resetButtonsMap();
        }
        if ((len > 3) && (buffer[3] == (uint8_t)simpleCommands_t::CMD_SAVE_NOW))
        {
            // save settings now
            userSettings::saveNow();
        }
        if ((len > 3) && (buffer[3] == (uint8_t)simpleCommands_t::CMD_REVERSE_LEFT_AXIS))
        {
            // change left axis polarity
            inputs::reverseLeftAxis();
        }
        if ((len > 3) && (buffer[3] == (uint8_t)simpleCommands_t::CMD_REVERSE_RIGHT_AXIS))
        {
            // change left axis polarity
            inputs::reverseRightAxis();
        }
        if ((len > 3) && (buffer[3] == (uint8_t)simpleCommands_t::CMD_SHOW_PIXELS))
        {
            // Show all pixels at once
            pixels::show();
        }
        if ((len > 3) && (buffer[3] == (uint8_t)simpleCommands_t::CMD_RESET_PIXELS))
        {
            // Turn off all pixels
            pixels::reset();
        }
        if ((len > 3) && (buffer[3] == (uint8_t)simpleCommands_t::CMD_ENCODER_PULSE_X1))
        {
            // Set pulse width to default
            inputs::setRotaryPulseX1();
        }
        if ((len > 3) && (buffer[3] == (uint8_t)simpleCommands_t::CMD_ENCODER_PULSE_X2))
        {
            // Set pulse width to double
            inputs::setRotaryPulseX2();
        }
        if ((len > 3) && (buffer[3] == (uint8_t)simpleCommands_t::CMD_ENCODER_PULSE_X3))
        {
            // Set pulse width to triple
            inputs::setRotaryPulseX3();
        }
        if ((len > 4) && (buffer[4] != 0xff))
        {
            // Set working mode of DPAD
            userSettings::setDPADWorkingMode((bool)buffer[4]);
        }
        // Note: byte index 5 is read-only
    }
    else if ((report_id == RID_FEATURE_BUTTONS_MAP) && (len >= BUTTONS_MAP_REPORT_SIZE))
    {
        if (buffer[0] <= MAX_INPUT_NUMBER)
        {
            selectedInput = buffer[0];
            if ((buffer[1] <= MAX_USER_INPUT_NUMBER) && (buffer[2] <= MAX_USER_INPUT_NUMBER))
                userSettings::setButtonMap(
                    (inputNumber_t)buffer[0],
                    (inputNumber_t)buffer[1],
                    (inputNumber_t)buffer[2]);
        }
    }
    else if ((report_id == RID_FEATURE_HARDWARE_ID) && (len >= HARDWARE_ID_REPORT_SIZE))
    {
        if ((factoryVID != UNDEFINED_ID) || (factoryPID != UNDEFINED_ID))
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
            {
                if ((vid == 0) && (pid == 0))
                    hidImplementation::common::clearStoredHardwareID();
                else
                    hidImplementation::common::storeHardwareID(vid, pid);
            } // else ignore
        } // else ignore
    }
    else
        log_e("Set feature report ID %u: ignored. Size: %u.", report_id, len);
}

// ----------------------------------------------------------------------------
// Output reports
// ----------------------------------------------------------------------------

void hidImplementation::common::onOutput(
    uint8_t report_id,
    const uint8_t *buffer,
    uint16_t len)
{
    // TODO: security lock
    if ((report_id == RID_OUTPUT_POWERTRAIN) && (len >= POWERTRAIN_REPORT_SIZE))
    {
        notify::telemetryData.powertrain.gear = (char)buffer[0];
        notify::telemetryData.powertrain.rpm = *((uint16_t *)(buffer + 1));
        notify::telemetryData.powertrain.rpmPercent = buffer[3];
        if (notify::telemetryData.powertrain.rpmPercent > 100)
            notify::telemetryData.powertrain.rpmPercent = 100;
        notify::telemetryData.powertrain.shiftLight1 = buffer[4];
        notify::telemetryData.powertrain.shiftLight2 = buffer[5];
        notify::telemetryData.powertrain.revLimiter = buffer[6];
        notify::telemetryData.powertrain.engineStarted = buffer[7];
        notify::telemetryData.powertrain.speed = *((uint16_t *)(buffer + 8));
    }
    else if ((report_id == RID_OUTPUT_ECU) && (len >= ECU_REPORT_SIZE))
    {
        notify::telemetryData.ecu.absEngaged = buffer[0];
        notify::telemetryData.ecu.tcEngaged = buffer[1];
        notify::telemetryData.ecu.drsEngaged = buffer[2];
        notify::telemetryData.ecu.pitLimiter = buffer[3];
        notify::telemetryData.ecu.lowFuelAlert = buffer[4];
        notify::telemetryData.ecu.absLevel = buffer[5];
        notify::telemetryData.ecu.tcLevel = buffer[6];
        notify::telemetryData.ecu.tcCut = buffer[7];
        notify::telemetryData.ecu.brakeBias = buffer[8];
        if (notify::telemetryData.ecu.brakeBias > 100)
            notify::telemetryData.ecu.brakeBias = 100;
    }
    else if ((report_id == RID_OUTPUT_RACE_CONTROL) && (len >= RACE_CONTROL_REPORT_SIZE))
    {
        notify::telemetryData.raceControl.blackFlag = buffer[0];
        notify::telemetryData.raceControl.blueFlag = buffer[1];
        notify::telemetryData.raceControl.checkeredFlag = buffer[2];
        notify::telemetryData.raceControl.greenFlag = buffer[3];
        notify::telemetryData.raceControl.orangeFlag = buffer[4];
        notify::telemetryData.raceControl.whiteFlag = buffer[5];
        notify::telemetryData.raceControl.yellowFlag = buffer[6];
        notify::telemetryData.raceControl.remainingLaps = *((uint16_t *)(buffer + 7));
        notify::telemetryData.raceControl.remainingMinutes = *((uint16_t *)(buffer + 9));
    }
    else if ((report_id == RID_OUTPUT_GAUGES) && (len >= GAUGES_REPORT_SIZE))
    {
        notify::telemetryData.gauges.relativeTurboPressure = buffer[0];
        if (notify::telemetryData.gauges.relativeTurboPressure > 100)
            notify::telemetryData.gauges.relativeTurboPressure = 100;
        notify::telemetryData.gauges.absoluteTurboPressure = static_cast<float>(*((uint16_t *)(buffer + 1)) / 100.0);
        notify::telemetryData.gauges.waterTemperature = *((uint16_t *)(buffer + 3));
        notify::telemetryData.gauges.oilPressure = static_cast<float>(*((uint16_t *)(buffer + 5)) / 100.0);
        notify::telemetryData.gauges.oilTemperature = *((uint16_t *)(buffer + 7));
        notify::telemetryData.gauges.relativeRemainingFuel = buffer[9];
        if (notify::telemetryData.gauges.relativeRemainingFuel > 100)
            notify::telemetryData.gauges.relativeRemainingFuel = 100;
        notify::telemetryData.gauges.absoluteRemainingFuel = *((uint16_t *)(buffer + 10));
    }
    else if ((report_id == RID_OUTPUT_PIXEL) && (len >= PIXEL_REPORT_SIZE))
    {
        if (buffer[0] > (uint8_t)pixelGroup_t::GRP_INDIVIDUAL)
            // Invalid pixel group
            return;

        pixels::set(
            (pixelGroup_t)buffer[0],
            buffer[1],
            buffer[4],
            buffer[3],
            buffer[2]);
        return;
    }
    notify::telemetryData.frameID = notify::telemetryData.frameID + 1;
    // log_d("frame id: %u", notify::telemetryData.frameID);
}

// ----------------------------------------------------------------------------
// Input reports
// ----------------------------------------------------------------------------

void hidImplementation::common::onReset(uint8_t *report)
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

// ----------------------------------------------------------------------------

void hidImplementation::common::onReportInput(
    uint8_t *report,
    bool &notifyConfigChanges,
    inputBitmap_t &inputsLow,
    inputBitmap_t &inputsHigh,
    uint8_t &POVstate,
    clutchValue_t &leftAxis,
    clutchValue_t &rightAxis,
    clutchValue_t &clutchAxis)
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