/**
 * @file CodedSwitchesTest.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-06-30
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

void send(uint64_t bitmap)
{
    DecouplingEvent event;
    event.rawInputBitmap = bitmap;
    event.rawInputChanges = bitmap;
    event.leftAxisValue = 0;
    event.rightAxisValue = 0;
    internals::inputHub::onRawInput(event);
}

//------------------------------------------------------------------
// MOCKS (static)
//------------------------------------------------------------------

uint64_t currentLow = 0ULL;

void internals::hid::reset()
{
    currentLow = 0ULL;
}

void internals::hid::reportInput(
    uint64_t inputsLow,
    uint64_t inputsHigh,
    uint8_t POVstate,
    uint8_t leftAxis,
    uint8_t rightAxis,
    uint8_t clutchAxis)
{
    currentLow = inputsLow;
}

//------------------------------------------------------------------

void internals::inputMap::map(
    bool isAltModeEngaged,
    uint64_t firmware_bitmap,
    uint64_t &low,
    uint64_t &high)
{
    if (isAltModeEngaged)
    {
        low = 0ULL;
        high = firmware_bitmap;
    }
    else
    {
        low = firmware_bitmap;
        high = 0ULL;
    }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
// Entry point
//------------------------------------------------------------------
//------------------------------------------------------------------

int main()
{
    std::cout << ("- Test initialization -") << std::endl;
    // NOTE: randomly selected input numbers
    InputNumber A1 = 0;
    InputNumber A2 = 1;
    InputNumber A4 = 2;
    InputNumber B1 = 8;
    InputNumber B2 = 9;
    InputNumber B4 = 10;
    InputNumber B8 = 12;

    // Book input numbers as the inputs namespace would do
    A1.book();
    A2.book();
    A4.book();
    B1.book();
    B2.book();
    B4.book();
    B8.book();

    // Create test specifications for two coded switches
    // Note that some input numbers are reused but others not
    CodedSwitch8 spec1;
    for (int i = 0; i < 7; i++)
        spec1.at(i) = 20 + i;
    spec1[7] = A1;
    CodedSwitch16 spec2;
    for (int i = 0; i < 15; i++)
        spec2.at(i) = 30 + i;
    spec2[15] = A2;
    CodedSwitch32 spec3;

    // Create valid coded switches
    inputHub::codedSwitch::add(A1, A2, A4, spec1);
    inputHub::codedSwitch::add(B1, B2, B4, B8, spec2);
    std::cout << ("- Create coded switches -") << std::endl;

    // Try to create invalid coded switches due to repeated binary-coded input numbers
    try
    {
        inputHub::codedSwitch::add(30, 31, 31, spec1);
        assert(false && "repeated input number accepted (1)");
    }
    catch (std::runtime_error)
    {
    }
    try
    {
        inputHub::codedSwitch::add(30, 31, 32, 32, spec2);
        assert(false && "repeated input number accepted (2)");
    }
    catch (std::runtime_error)
    {
    }
    try
    {
        inputHub::codedSwitch::add(30, 31, 32, 30, 34, spec3);
        assert(false && "repeated input number accepted (3)");
    }
    catch (std::runtime_error)
    {
    }

    // Try to create invalid coded switches due to binary-coded input numbers already used
    try
    {
        inputHub::codedSwitch::add(30, 31, A4, spec1);
        assert(false && "reused input number accepted (1)");
    }
    catch (std::runtime_error)
    {
    }
    try
    {
        inputHub::codedSwitch::add(30, 31, B1, 32, spec2);
        assert(false && "reused input number accepted (2)");
    }
    catch (std::runtime_error)
    {
    }
    try
    {
        inputHub::codedSwitch::add(B4, 30, 31, 32, 33, spec3);
        assert(false && "reused input number accepted (3)");
    }
    catch (std::runtime_error)
    {
    }

    // Try to create invalid coded switches due to unknown input numbers
    try
    {
        inputHub::codedSwitch::add(UNSPECIFIED::VALUE, 30, 31, spec1);
        assert(false && "unknown input number accepted (1)");
    }
    catch (std::runtime_error)
    {
    }
    try
    {
        inputHub::codedSwitch::add(30, UNSPECIFIED::VALUE, 30, 31, spec2);
        assert(false && "unknown input number accepted (2)");
    }
    catch (std::runtime_error)
    {
    }
    try
    {
        inputHub::codedSwitch::add(30, 31, 32, 33, UNSPECIFIED::VALUE, spec3);
        assert(false && "unknown input number accepted (3)");
    }
    catch (std::runtime_error)
    {
    }

    // Initialize the firmware code
    std::cout << ("- Initializing inputHub -") << std::endl;
    internals::inputHub::getReady();
    OnStart::notify();

    // Check the booked input numbers
    std::cout << ("- Check input numbers -") << std::endl;
    // std::cout << (uint64_t)InputNumber::booked() << std::endl;
    assert(InputNumber::booked(A1) && "Input number A1 should be booked");
    assert(InputNumber::booked(A2) && "Input number A2 should be booked");
    assert(!InputNumber::booked(A4) && "Input number A4 should NOT be booked");
    assert(!InputNumber::booked(B1) && "Input number B1 should NOT be booked");
    assert(InputNumber::booked(20) && "Input number 20 should be booked");
    assert(InputNumber::booked(26) && "Input number 26 should be booked");
    assert(InputNumber::booked(30) && "Input number 30 should be booked");
    assert(InputNumber::booked(44) && "Input number 44 should be booked");

    // Check input in binary code
    std::cout << ("- Check coded switch A (8 positions) -") << std::endl;
    send(0);
    for (uint8_t posA = 0; posA < 8; posA++)
    {
        // std::cout << (int)posA << std::endl;
        send(posA);
        assert<uint64_t>::equals("decoded switch A", (uint64_t)spec1.at(posA) | (uint64_t)spec2.at(0), currentLow);
    }

    std::cout << ("- Check coded switch B (16 positions) -") << std::endl;
    send(0);
    for (uint8_t posB = 0; posB < 16; posB++)
    {
        uint64_t input = posB << (uint8_t)B1;
        bool mostSignificantBit = (input & 0b0100000000000);
        input &= 0b0011111111111;
        if (mostSignificantBit)
            input |= 0b1000000000000;

        // std::cout << (int)posB << " -> " << (input) << std::endl;
        send(input);
        assert<uint64_t>::equals("decoded switch B", (uint64_t)spec2.at(posB) | (uint64_t)spec1.at(0), currentLow);
    }

    // Check that unrelated inputs are not removed
    std::cout << ("- Check unrelated inputs -") << std::endl;
    uint64_t base = 0;
    uint64_t expected_base = (uint64_t)spec1.at(0) | (uint64_t)spec2.at(0);
    InputNumber ur1 = 11;
    InputNumber ur2 = 3;
    InputNumber ur3 = 63;
    send(base | (uint64_t)ur1);
    assert<uint64_t>::equals("1", expected_base | (uint64_t)ur1, currentLow);
    send(base | (uint64_t)ur2);
    assert<uint64_t>::equals("2", expected_base | (uint64_t)ur2, currentLow);
    send(base | (uint64_t)ur3);
    assert<uint64_t>::equals("3", expected_base | (uint64_t)ur3, currentLow);
    base = 0b1011000000001;
    expected_base = (uint64_t)spec1.at(1) | (uint64_t)spec2.at(14);
    send(base | (uint64_t)ur1 | (uint64_t)ur2 | (uint64_t)ur3);
    assert<uint64_t>::equals("4", expected_base | (uint64_t)ur1 | (uint64_t)ur2 | (uint64_t)ur3, currentLow);
}
