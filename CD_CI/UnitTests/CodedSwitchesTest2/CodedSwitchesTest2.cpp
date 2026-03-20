/**
 * @file CodedSwitchesTest2.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2026-03-20
 * @brief Unit test
 *
 * @copyright Licensed under the EUPL
 *
 */

//------------------------------------------------------------------
// Imports
//------------------------------------------------------------------

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include "cd_ci_assertions.hpp"
#include <iostream>

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

void send(uint128_t bitmap)
{
    DecouplingEvent event;
    event.rawInputBitmap = bitmap;
    event.leftAxisValue = 0;
    event.rightAxisValue = 0;
    internals::inputHub::onRawInput(event);
}

#define TEST_IN 64
static uint128_t test_bitmap{}; // Initialized later
static uint128_t on1_off2{};    // Initialized later
static uint128_t on2_off1{};    // Initialized later
static uint128_t both_on{};      // Initialized later
static uint128_t all_off{};

//------------------------------------------------------------------
// MOCKS (static)
//------------------------------------------------------------------

uint128_t current{};

void internals::hid::reset()
{
    current = {};
}

void internals::hid::reportInput(
    const uint128_t &inputs,
    uint8_t POVstate,
    uint8_t leftAxis,
    uint8_t rightAxis,
    uint8_t clutchAxis)
{
    current = inputs;
}

//------------------------------------------------------------------

void internals::inputMap::map(
    bool isAltModeEngaged,
    uint128_t &bitmap)
{
    // do nothing
}

//------------------------------------------------------------------
//------------------------------------------------------------------
// Entry point
//------------------------------------------------------------------
//------------------------------------------------------------------

int main()
{
    std::cout << ("- Test initialization -") << std::endl;
    test_bitmap.set_bit(TEST_IN, true);
    // NOTE: randomly selected input numbers
    InputNumber A1 = 0;
    InputNumber A2 = 1;
    InputNumber A4 = 2;
    InputNumber B1 = 8;
    InputNumber B2 = 9;
    InputNumber B4 = 10;

    // Book input numbers as the inputs namespace would do
    A1.book();
    A2.book();
    A4.book();
    B1.book();
    B2.book();
    B4.book();

    // Create test specifications for two coded switches
    // Note that some input numbers are reused but others not
    CodedSwitch8 spec1;
    CodedSwitch8 spec2;

    // Assign the very same input number to a position in each switch
    // As other positions have no assignment,
    // no input should be detected ("all off")
    spec1[1] = 64;
    spec2[7] = 64;

    // Create position selection bitmaps
    on1_off2.set_bit(A1, true);
    on2_off1.set_bit(B1, true);
    on2_off1.set_bit(B2, true);
    on2_off1.set_bit(B4, true);
    both_on.set_bit(A1, true);
    both_on.set_bit(B1, true);
    both_on.set_bit(B2, true);
    both_on.set_bit(B4, true);

    // Create coded switches
    inputHub::codedSwitch::add(A1, A2, A4, spec1);
    inputHub::codedSwitch::add(B1, B2, B4, spec2);

    // Initialize the firmware code
    std::cout << ("- Initializing inputHub -") << std::endl;
    internals::inputHub::getReady();
    OnStart::notify();

    // Check the booked input numbers
    std::cout << ("- Check input numbers -") << std::endl;
    assert(!InputNumber::booked(A1) && "Input number A1 should be booked");
    assert(!InputNumber::booked(A2) && "Input number A2 should be booked");
    assert(!InputNumber::booked(A4) && "Input number A4 should NOT be booked");
    assert(!InputNumber::booked(B1) && "Input number B1 should NOT be booked");
    assert(!InputNumber::booked(B2) && "Input number B1 should NOT be booked");
    assert(!InputNumber::booked(B4) && "Input number B1 should NOT be booked");
    assert(InputNumber::booked(TEST_IN) &&
           "Test input number should be booked");

    std::cout << ("- Test -") << std::endl;
    // Check all off
    send(all_off);
    assert(current == all_off);

    // Move switch 1 to position 1
    send(on1_off2);
    assert(current == test_bitmap);

    // Move switch 2 to position 7
    send(both_on);
    assert(current == test_bitmap);

    // Move switch 1 to position 0
    send(on2_off1);
    assert(current == test_bitmap);

    // Move both switches to position 0
    send(all_off);
    assert(current == all_off);

    // Move switch 2 to position 7
    send(on2_off1);
    assert(current == test_bitmap);

    // Move switch 1 to position 1
    send(both_on);
    assert(current == test_bitmap);

    // Move switch 2 to position 0
    send(on1_off2);
    assert(current == test_bitmap);

    return 0;
}
