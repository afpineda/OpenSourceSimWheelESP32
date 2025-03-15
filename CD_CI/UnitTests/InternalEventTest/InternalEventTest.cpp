/**
 * @file InternalEventTest.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-04
 * @brief Unit test
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "InternalTypes.hpp"

#include <iostream>

uint8_t witness1 = 0;
uint8_t witness2 = 0;

void void_callback1()
{
    witness1 = 66;
}

void void_callback2()
{
    witness2 = 66;
}

void int_callback1(uint8_t value)
{
    witness1 = value;
}

void int_callback2(uint8_t value)
{
    witness2 = value;
}

void test1()
{
    std::cout << "- Test 1 -" << std::endl;
    witness1 = 0;
    witness2 = 0;
    OnStart::notify();
    assert((witness1==66) && "void_callback1() not called");
    assert((witness2==66) && "void_callback2() not called");
}

void test2()
{
    std::cout << "- Test 2 -" << std::endl;
    witness1 = 0;
    witness2 = 0;
    OnBitePoint::notify(99);
    assert((witness1==99) && "int_callback1() not called");
    assert((witness2==99) && "int_callback2() not called");
}


int main()
{
    OnStart::subscribe(void_callback1);
    OnStart::subscribe(void_callback2);
    OnBitePoint::subscribe(int_callback1);
    OnBitePoint::subscribe(int_callback2);

    test1();
    test2();
}
