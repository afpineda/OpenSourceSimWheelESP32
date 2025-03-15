/**
 * @file GPIOTest.cpp
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
    std::cout << "- Test 1 -" << std::endl;
    GPIO g1;
    assert((g1 == UNSPECIFIED::VALUE) && "GPIO not created as 'not connected'");
    GPIO g2(UNSPECIFIED::VALUE);
    assert((g2 == UNSPECIFIED::VALUE) && "GPIO not assigned to 'not connected'");
}

void test2()
{
    std::cout << "- Test 2 -" << std::endl;
    try
    {
        GPIO g1 = TEST_RESERVED_GPIO;
        assert(false && "Reserved GPIO was accepted");
    }
    catch (gpio_error e)
    {
    }

    GPIO g2 = 40; // No exception expected

    try
    {
        GPIO g1 = 223;
        assert(false && "Non-existing GPIO was accepted");
    }
    catch (gpio_error e)
    {
    }
}

void test3()
{
    std::cout << "- Test 3 -" << std::endl;
    GPIO g1;
    int asInt;
    g1 = UNSPECIFIED::VALUE;
    asInt = g1;
    assert((asInt == -1) && "Implicit cast failed (1)");

    g1 = 16;
    asInt = g1;
    assert((asInt == 16) && "Implicit cast failed (2)");
}

void test4()
{
    std::cout << "- Test 4 -" << std::endl;
    try
    {
        GPIO g;
        g.abortIfUnspecified();
        assert(false && "Non-existing GPIO was accepted");
    }
    catch (gpio_error e)
    {
    }

    GPIO g1 = 4;
    g1.reserve();
    try
    {
        GPIO g = 4;
        g.reserve();
        assert(false && "Reserved GPIO was accepted");
    }
    catch (gpio_error e)
    {
    }
}


void test5()
{
    std::cout << "- Test 5 -" << std::endl;
    try
    {
        OutputGPIO g = TEST_NO_OUTPUT_GPIO;
        assert(false && "Non-output-capable GPIO was accepted");
    }
    catch (gpio_error e)
    {
    }

    try
    {
        RTC_GPIO g = TEST_RTC_GPIO1-1;
        assert(false && "Non-rtc-capable GPIO was accepted");
    }
    catch (gpio_error e)
    {
    }
}


int main()
{
    test1();
    test2();
    test3();
    test4();
    test5();
}