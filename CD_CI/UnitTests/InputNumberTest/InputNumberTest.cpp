/**
 * @file InputNumberTest.cpp
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
        InputNumber x = 128;
        assert(false && "128 assignment was valid");
    }
    catch (const invalid_input_number e)
    {
    }

    try
    {
        InputNumber x = 240;
        assert(false && "240 assignment was valid");
    }
    catch (const invalid_input_number e)
    {
    }

    i1 = 0;   // No exception expected
    i1 = 127; // No exception expected

    InputNumber i2 = i1;
    assert((i1 == i2) && "Copy operator failed");

    uint8_t asByte = i1;
    assert((asByte == 127) && "Implicit typecast failed");
}

void test2()
{
    std::cout << "- Test 2-" << std::endl;
    InputNumber i1 = 0;
    uint128_t mask = (uint128_t)i1;
    assert((mask.low == 1) && "Explicit typecast 1 failed");
    i1 = 63;
    mask = (uint128_t)i1;
    assert((mask.low == 0x8000000000000000) && "Explicit typecast 2 failed");
}

void test3()
{
    std::cout << "- Test 3-" << std::endl;
    InputNumberCombination i1 = {127, 63, 0};
    assert((i1.low == 0x8000000000000001));
    assert((i1.high == 0x8000000000000000));

    InputNumberCombination i2(uint128_t::neg());
    assert(i2.low == 0xFFFFFFFFFFFFFFFF);
    assert(i2.high == 0xFFFFFFFFFFFFFFFF);

    InputNumberCombination i3;
    i3 = uint128_t::neg();
    assert(i3.low == 0xFFFFFFFFFFFFFFFF);
    assert(i3.high == 0xFFFFFFFFFFFFFFFF);
}

void test4()
{
    std::cout << "- Test 4-" << std::endl;
    InputNumber i1 = 0;
    InputNumber i2 = 63;
    i1.book();
    assert(InputNumber::booked(0) && "book(0) failed");
    i2.book();
    assert(InputNumber::booked(63) && "book(63) failed");

    uint128_t mask = InputNumber::booked();
    assert((mask.low == 0x8000000000000001) && "Wrong booked I.N.");
}

int main()
{
    test1();
    test2();
    test3();
    test4();

    return 0;
}