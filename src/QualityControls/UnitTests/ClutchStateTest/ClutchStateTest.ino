/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-03-03
 * @brief Unit Test. See [README](./README.md)
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include "SimWheel.h"
#include "SimWheelTypes.h"
#include "debugUtils.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

#define STR_FUNCTION "function"
#define STR_L_AXIS "left axis"
#define STR_R_AXIS "right axis"
#define STR_C_AXIS "combined axis"
#define STR_CAL "calibration in progress"
#define STR_BITEP "bite point"
#define STR_ALT "ALT request"

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

template <typename T>
void assertEquals(const char *text, T expected, T found)
{
    if (expected != found)
    {
        serialPrintf("[assertEquals] (%s). Expected: %d, found: %d\n", text, expected, found);
    }
}

template <typename T>
void assertNonEquals(const char *text, T expected, T found)
{
    if (expected == found)
    {
        serialPrintf("[assertNonEquals] (%s). Expected: %d, found: %d\n", text, expected, found);
    }
}

void assertAlmostEquals(const char *text, clutchValue_t expected, clutchValue_t found)
{
    int diff = expected-found;
    if ((diff<1) || (diff>1))
    {
        serialPrintf("[assertAlmostEquals] (%s). Expected: %d, found: %d\n", text, expected, found);
    }
}

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

void hidImplementation::reportChangeInConfig()
{

}

void notify::bitePoint(clutchValue_t n)
{

}


//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    esp_log_level_set("*", ESP_LOG_ERROR);
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("-- READY --");
    Serial.println("-- GO --");

    // Bite point
    Serial.println("-- Set bite point");
    clutchState::setBitePoint(CLUTCH_DEFAULT_VALUE);
    clutchState::setBitePoint(CLUTCH_1_4_VALUE);
    assertEquals<clutchValue_t>(STR_BITEP, CLUTCH_1_4_VALUE, clutchState::bitePoint);

    // Button function
    Serial.println("-- CF_BUTTON");
    clutchState::setFunction(CF_BUTTON);
    assertEquals<clutchFunction_t>(STR_FUNCTION, CF_BUTTON, clutchState::currentFunction);
    clutchState::setLeftAxis(CLUTCH_1_4_VALUE);
    clutchState::setRightAxis(CLUTCH_3_4_VALUE);
    assertEquals<clutchValue_t>(STR_L_AXIS, CLUTCH_NONE_VALUE, clutchState::leftAxis);
    assertEquals<clutchValue_t>(STR_R_AXIS, CLUTCH_NONE_VALUE, clutchState::rightAxis);
    assertEquals<clutchValue_t>(STR_C_AXIS, CLUTCH_NONE_VALUE, clutchState::combinedAxis);
    assertEquals<bool>(STR_ALT, false, clutchState::isALTRequested());
    assertEquals<bool>(STR_CAL, false, clutchState::isCalibrationInProgress());

    // ALT function
    Serial.println("-- CF_ALT");
    clutchState::setFunction(CF_ALT);
    clutchState::setLeftAxis(CLUTCH_1_4_VALUE);
    clutchState::setRightAxis(CLUTCH_NONE_VALUE);
    assertEquals<clutchFunction_t>(STR_FUNCTION, CF_ALT, clutchState::currentFunction);
    assertEquals<clutchValue_t>(STR_L_AXIS, CLUTCH_NONE_VALUE, clutchState::leftAxis);
    assertEquals<clutchValue_t>(STR_R_AXIS, CLUTCH_NONE_VALUE, clutchState::rightAxis);
    assertEquals<clutchValue_t>(STR_C_AXIS, CLUTCH_NONE_VALUE, clutchState::combinedAxis);
    assertEquals<bool>(STR_CAL, false, clutchState::isCalibrationInProgress());
    assertEquals<bool>(STR_ALT, false, clutchState::isALTRequested());

    Serial.println("-- CF_ALT: left on, right off");
    clutchState::setLeftAxis(CLUTCH_3_4_VALUE);
    clutchState::setRightAxis(CLUTCH_NONE_VALUE);
    assertEquals<bool>(STR_ALT, true, clutchState::isALTRequested());
    Serial.println("-- CF_ALT: left off, right on");
    clutchState::setLeftAxis(CLUTCH_1_4_VALUE);
    clutchState::setRightAxis(CLUTCH_FULL_VALUE);
    assertEquals<bool>(STR_ALT, true, clutchState::isALTRequested());
    Serial.println("-- CF_ALT: left on, right on");
    clutchState::setLeftAxis(CLUTCH_FULL_VALUE);
    clutchState::setRightAxis(CLUTCH_3_4_VALUE);
    assertEquals<bool>(STR_ALT, true, clutchState::isALTRequested());
    Serial.println("-- CF_ALT: left off, right off");
    clutchState::setLeftAxis(CLUTCH_NONE_VALUE);
    clutchState::setRightAxis(CLUTCH_1_4_VALUE);
    assertEquals<bool>(STR_ALT, false, clutchState::isALTRequested());

    // AXIS function
    Serial.println("-- CF_AXIS");
    clutchState::setFunction(CF_AXIS);
    assertEquals<clutchFunction_t>(STR_FUNCTION, CF_AXIS, clutchState::currentFunction);
    clutchState::setLeftAxis(CLUTCH_1_4_VALUE);
    clutchState::setRightAxis(CLUTCH_3_4_VALUE);
    assertEquals<clutchValue_t>(STR_L_AXIS, CLUTCH_1_4_VALUE, clutchState::leftAxis);
    assertEquals<clutchValue_t>(STR_R_AXIS, CLUTCH_3_4_VALUE, clutchState::rightAxis);
    assertEquals<clutchValue_t>(STR_C_AXIS, CLUTCH_NONE_VALUE, clutchState::combinedAxis);
    assertEquals<bool>(STR_ALT, false, clutchState::isALTRequested());
    assertEquals<bool>(STR_CAL, false, clutchState::isCalibrationInProgress());

    Serial.println("-- CF_AXIS: new bite point");
    clutchState::setBitePoint(CLUTCH_DEFAULT_VALUE);
    assertEquals<clutchValue_t>(STR_L_AXIS, CLUTCH_1_4_VALUE, clutchState::leftAxis);
    assertEquals<clutchValue_t>(STR_R_AXIS, CLUTCH_3_4_VALUE, clutchState::rightAxis);
    assertEquals<clutchValue_t>(STR_C_AXIS, CLUTCH_NONE_VALUE, clutchState::combinedAxis);

    // CLUTCH FUNCTION
    Serial.println("-- CF_CLUTCH");
    clutchState::setFunction(CF_CLUTCH);
    assertEquals<clutchFunction_t>(STR_FUNCTION, CF_CLUTCH, clutchState::currentFunction);
    assertEquals<clutchValue_t>(STR_L_AXIS, CLUTCH_NONE_VALUE, clutchState::leftAxis);
    assertEquals<clutchValue_t>(STR_R_AXIS, CLUTCH_NONE_VALUE, clutchState::rightAxis);
    assertNonEquals<clutchValue_t>(STR_C_AXIS, CLUTCH_NONE_VALUE, clutchState::combinedAxis);
    assertEquals<bool>(STR_ALT, false, clutchState::isALTRequested());

    Serial.println("-- CF_CLUTCH: left off, right off");
    clutchState::setLeftAxis(CLUTCH_NONE_VALUE);
    clutchState::setRightAxis(CLUTCH_NONE_VALUE);
    assertEquals<clutchValue_t>(STR_L_AXIS, CLUTCH_NONE_VALUE, clutchState::leftAxis);
    assertEquals<clutchValue_t>(STR_R_AXIS, CLUTCH_NONE_VALUE, clutchState::rightAxis);
    assertEquals<clutchValue_t>(STR_C_AXIS, CLUTCH_NONE_VALUE, clutchState::combinedAxis);
    assertEquals<bool>(STR_CAL, false, clutchState::isCalibrationInProgress());

    Serial.println("-- CF_CLUTCH: left on, right off");
    clutchState::setBitePoint(CLUTCH_DEFAULT_VALUE);
    clutchState::setLeftAxis(CLUTCH_FULL_VALUE);
    clutchState::setRightAxis(CLUTCH_NONE_VALUE);
    assertEquals<clutchValue_t>(STR_L_AXIS, CLUTCH_NONE_VALUE, clutchState::leftAxis);
    assertEquals<clutchValue_t>(STR_R_AXIS, CLUTCH_NONE_VALUE, clutchState::rightAxis);
    assertAlmostEquals(STR_C_AXIS, clutchState::bitePoint, clutchState::combinedAxis);
    assertEquals<clutchValue_t>(STR_BITEP, CLUTCH_DEFAULT_VALUE, clutchState::bitePoint);
    assertEquals<bool>(STR_CAL, true, clutchState::isCalibrationInProgress());

    Serial.println("-- CF_CLUTCH: left on, right off, new bite point");
    clutchState::setBitePoint(CLUTCH_3_4_VALUE);
    assertAlmostEquals(STR_C_AXIS, clutchState::bitePoint, clutchState::combinedAxis);
    assertEquals<clutchValue_t>(STR_BITEP, CLUTCH_3_4_VALUE, clutchState::bitePoint);

    Serial.println("-- CF_CLUTCH: left off, right on");
    clutchState::setBitePoint(CLUTCH_DEFAULT_VALUE);
    clutchState::setLeftAxis(CLUTCH_NONE_VALUE);
    clutchState::setRightAxis(CLUTCH_FULL_VALUE);
    assertEquals<clutchValue_t>(STR_L_AXIS, CLUTCH_NONE_VALUE, clutchState::leftAxis);
    assertEquals<clutchValue_t>(STR_R_AXIS, CLUTCH_NONE_VALUE, clutchState::rightAxis);
    assertAlmostEquals(STR_C_AXIS, clutchState::bitePoint, clutchState::combinedAxis);
    assertEquals<clutchValue_t>(STR_BITEP, CLUTCH_DEFAULT_VALUE, clutchState::bitePoint);
    assertEquals<bool>(STR_CAL, true, clutchState::isCalibrationInProgress());

    Serial.println("-- CF_CLUTCH: left off, right on, new bite point");
    clutchState::setBitePoint(CLUTCH_1_4_VALUE);
    assertAlmostEquals(STR_C_AXIS, clutchState::bitePoint, clutchState::combinedAxis);
    assertEquals<clutchValue_t>(STR_BITEP, CLUTCH_1_4_VALUE, clutchState::bitePoint);

    Serial.println("-- CF_CLUTCH: left on, right on, new bite point");
    clutchState::setLeftAxis(CLUTCH_FULL_VALUE);
    clutchState::setRightAxis(CLUTCH_FULL_VALUE);
    assertEquals<clutchValue_t>(STR_L_AXIS, CLUTCH_NONE_VALUE, clutchState::leftAxis);
    assertEquals<clutchValue_t>(STR_R_AXIS, CLUTCH_NONE_VALUE, clutchState::rightAxis);
    assertEquals<clutchValue_t>(STR_C_AXIS, CLUTCH_FULL_VALUE, clutchState::combinedAxis);
    assertEquals<bool>(STR_CAL, false, clutchState::isCalibrationInProgress());
    clutchState::setBitePoint(CLUTCH_DEFAULT_VALUE);
    assertEquals<clutchValue_t>(STR_C_AXIS, CLUTCH_FULL_VALUE, clutchState::combinedAxis);

    Serial.println("-- END --");
}

void loop()
{
    vTaskDelay(DEBOUNCE_TICKS);
}