/**
 * @file Report4Test.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-03-01
 * @brief Integration test
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
#include <cinttypes>
#include <cassert>

//-------------------------------------------------------------------
// Auxiliary
//-------------------------------------------------------------------

typedef struct __attribute__((packed))
{
    uint8_t fw = 0xFF;
    uint8_t noAlt = 0xFF;
    uint8_t alt = 0xFF;
} Report4;

#define REPORT4BYTES(s) ((uint8_t *)&s)

//-------------------------------------------------------------------
// Mocks
//-------------------------------------------------------------------

extern uint8_t selectedInput;

static Report4 received;

class InputMapMock : public InputMapService
{
public:
    inline static bool _saved = false;
    inline static bool _loaded = false;

    virtual void setMap(
        uint8_t firmware_defined,
        uint8_t user_defined,
        uint8_t user_defined_alt)
    {
        received.fw = firmware_defined;
        received.noAlt = user_defined;
        received.alt = user_defined_alt;
    }

    virtual void getMap(
        uint8_t firmware_defined,
        uint8_t &user_defined,
        uint8_t &user_defined_alt)
    {
        user_defined = received.noAlt;
        user_defined_alt = received.alt;
    }
} inputMapMock;

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------
//-------------------------------------------------------------------

int main()
{
    assert((sizeof(Report4) == BUTTONS_MAP_REPORT_SIZE) && "Test is outdated");
    InputMapService::inject(&inputMapMock);

    Report4 r4;

    // Select an invalidad input id
    r4.fw = 0xFe;
    r4.alt = 0;
    r4.noAlt = 0;
    internals::hid::common::onSetFeature(RID_FEATURE_BUTTONS_MAP, REPORT4BYTES(r4), sizeof(Report4));
    assert((selectedInput > 127) && "Input should not be selected");
    assert(
        ((received.fw == 0xFF) && (received.alt == 0xFF) && (received.noAlt == 0xFF)) &&
        "map should not be set (1)");

    // Select a valid input id, but do not overwrite map
    r4.fw = 12;
    r4.alt = 0xFE;
    r4.noAlt = 0xFA;
    internals::hid::common::onSetFeature(RID_FEATURE_BUTTONS_MAP, REPORT4BYTES(r4), sizeof(Report4));
    assert((selectedInput == 12) && "Input should be selected");
    assert(
        ((received.fw == 0xFF) && (received.alt == 0xFF) && (received.noAlt == 0xFF)) &&
        "map should not be set (2)");

    return 0;

    // Select a valid input id and overwrite map
    r4.fw = 20;
    r4.alt = 20;
    r4.noAlt = 80;
    internals::hid::common::onSetFeature(RID_FEATURE_BUTTONS_MAP, REPORT4BYTES(r4), sizeof(Report4));
    assert((selectedInput == 20) && "Input should be selected (2)");
    assert(
        ((received.fw == 20) && (received.alt == 20) && (received.noAlt == 80)) &&
        "map should be set");

     // Select a valid input id, then read
     received.fw = 0xFF;
     received.alt = 99;
     received.noAlt = 99;
     r4.fw = 33;
     r4.alt = 0xFE;
     r4.noAlt = 0xFA;
     internals::hid::common::onSetFeature(RID_FEATURE_BUTTONS_MAP, REPORT4BYTES(r4), sizeof(Report4));
     assert((selectedInput == 33) && "Input should be selected (3)");
     internals::hid::common::onGetFeature(RID_FEATURE_BUTTONS_MAP, REPORT4BYTES(r4), sizeof(Report4));
     assert(
        ((received.fw == 33) && (received.alt == 99) && (received.noAlt == 99)) &&
        "map not retrieved");
}
