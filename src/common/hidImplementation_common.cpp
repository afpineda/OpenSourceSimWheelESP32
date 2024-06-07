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

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

inputNumber_t selectedInput = UNSPECIFIED_INPUT_NUMBER;

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
        return CAPABILITIES_REPORT_SIZE;
    }
    if ((report_id == RID_FEATURE_CONFIG) && (len >= CONFIG_REPORT_SIZE))
    {

        buffer[0] = (uint8_t)userSettings::cpWorkingMode;
        buffer[1] = (uint8_t)userSettings::altButtonsWorkingMode;
        buffer[2] = (uint8_t)userSettings::bitePoint;
        buffer[3] = (uint8_t)power::getLastBatteryLevel();
        buffer[4] = (uint8_t)userSettings::dpadWorkingMode;
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
    return 0;
}

// ----------------------------------------------------------------------------

void hidImplementation::common::onSetFeature(uint8_t report_id, const uint8_t *buffer, uint16_t len)
{
    if (report_id == RID_FEATURE_CONFIG)
    {
        if ((len > 0) && (buffer[0] >= CF_CLUTCH) && (buffer[0] <= CF_BUTTON))
        {
            // clutch function
            userSettings::setCPWorkingMode((clutchFunction_t)buffer[0]);
        }
        if ((len > 1) && (buffer[1] != 0xff))
        {
            // ALT Buttons mode
            userSettings::setALTButtonsWorkingMode((bool)buffer[1]);
        }
        if ((len > 2) && ((clutchValue_t)buffer[2] >= CLUTCH_NONE_VALUE) && ((clutchValue_t)buffer[2] <= CLUTCH_FULL_VALUE))
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
        if ((len > 4) && (buffer[4] != 0xff))
        {
            // Set working mode of DPAD
            userSettings::setDPADWorkingMode((bool)buffer[4]);
        }
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
}

// ----------------------------------------------------------------------------
// Input  reports
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