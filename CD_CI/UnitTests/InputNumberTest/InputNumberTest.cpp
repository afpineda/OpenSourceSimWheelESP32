/**
 * @file BitQueueTest.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-10
 * @brief Unit test
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheelTypes.hpp"
#include <cassert>
#include <iostream>

void test1()
{
    std::cout << "- Test 1-" << std::endl;

    InputNumber i1;
    assert((i1 == UNSPECIFIED::VALUE) && "I.N. created with known value");

    try
    {
        InputNumber x = 64;
        assert(false && "64 assignment was valid");
    }
    catch (const invalid_input_number e)
    {
    }

    try
    {
        InputNumber x = 120;
        assert(false && "120 assignment was valid");
    }
    catch (const invalid_input_number e)
    {
    }

    i1 = 0;  // No exception expected
    i1 = 63; // No exception expected

    InputNumber i2 = i1;
    assert((i1 == i2) && "Copy operator failed");

    uint8_t asByte = i1;
    assert((asByte == 63) && "Implicit typecast failed");
}

void test2()
{
    std::cout << "- Test 2-" << std::endl;
    InputNumber i1 = 0;
    uint64_t mask = (uint64_t)i1;
    assert((mask == 1) && "Explicit typecast 1 failed");
    i1 = 63;
    mask = (uint64_t)i1;
    assert((mask == 0x8000000000000000) && "Explicit typecast 2 failed");
}

void test3()
{
    std::cout << "- Test 3-" << std::endl;
    InputNumberCombination i1 = {63,0};

    uint64_t mask = (uint64_t)i1;
    assert((mask == 0x8000000000000001) && "Explicit typecast 3 failed");
}

void test4()
{
    std::cout << "- Test 4-" << std::endl;
    InputNumber i1=0;
    InputNumber i2=63;
    i1.book();
    assert(InputNumber::booked(0) && "book(0) failed");
    i2.book();
    assert(InputNumber::booked(63) && "book(63) failed");

    uint64_t mask = InputNumber::booked();
    assert((mask == 0x8000000000000001) && "Wrong booked I.N.");
}

int main()
{
    test1();
    test2();
    test3();
    test4();
}