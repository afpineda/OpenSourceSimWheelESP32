/**
 * @file InputMapTest.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-10
 * @brief Unit test
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include <iostream>

bool loaded = false;

void reset()
{
    InputMapService::reset();
    OnStart::clearSubscriptions();
    loaded = false;
    internals::inputMap::clear();
}

void loadSettingsCallback(UserSetting setting)
{
    if (setting == UserSetting::INPUT_MAP)
        loaded = true;
}

void printMap(uint8_t a, uint8_t b, uint8_t c)
{
    std::cout << "Map(" << (int)a << ") = ";
    std::cout << (int)b << " , " << (int)c << std::endl;
}

void test1()
{
    std::cout << "- Test 1 -" << std::endl;
    uint8_t alt, noAlt;

    reset();
    internals::inputMap::getReady();
    OnStart::notify();

    assert((loaded) && "user map not loaded from storage");
    InputMapService::call::getMap(9, noAlt, alt);
    assert((noAlt == 9) && "default map not set (1)");
    assert((alt == 9 + 64) && "default map not set (2)");
    InputMapService::call::getMap(63, noAlt, alt);
    assert((noAlt == 63) && "default map not set (3)");
    assert((alt == 63 + 64) && "default map not set (4)");
}

void test2()
{
    std::cout << "- Test 2 -" << std::endl;
    uint8_t alt, noAlt;

    reset();
    inputMap::set(0, 1, 2);
    internals::inputMap::getReady();
    OnStart::notify();

    InputMapService::call::getMap(0, noAlt, alt);
    assert((noAlt == 1) && "custom firmware-defined map not respected (1)");
    assert((alt == 2) && "custom firmware-defined map not respected (2)");
    InputMapService::call::getMap(7, noAlt, alt);
    assert((noAlt == 7) && "default map not set (1)");
    assert((alt == 7 + 64) && "default map not set (2)");
}

void test3()
{
    std::cout << "- Test 3 -" << std::endl;
    uint8_t alt, noAlt;

    reset();
    internals::inputMap::getReady();
    OnStart::notify();

    InputMapService::call::setMap(99, 0, 1); // no exception should arise
    InputMapService::call::setMap(0, 128, 0);
    InputMapService::call::getMap(0, noAlt, alt);
    assert((noAlt == 0) && "setMap failed (1)");
    assert((alt == 64) && "setMap failed (2)");
    InputMapService::call::setMap(0, 0, 220);
    InputMapService::call::getMap(0, noAlt, alt);
    assert((noAlt == 0) && "setMap failed (3)");
    assert((alt == 64) && "setMap failed (4)");
    InputMapService::call::setMap(0, 63, 127);
    InputMapService::call::getMap(0, noAlt, alt);
    assert((noAlt == 63) && "setMap failed (5)");
    assert((alt == 127) && "setMap failed (6)");
}

#define BMPL(n) (1ULL << (n))
#define BMPH(n) (1ULL << (n - 64))

void test4()
{
    std::cout << "- Test 4 -" << std::endl;
    uint8_t alt, noAlt;
    uint64_t rawBitmap, low, high;

    reset();
    inputMap::set(0, 64, 127);
    inputMap::set(1, 64, 127);
    inputMap::set(2, 0, 1);
    internals::inputMap::getReady();
    OnStart::notify();

    rawBitmap = 0ULL;
    internals::inputMap::map(false, rawBitmap, low, high);
    // std::cout << low << "-" << high << std::endl;
    assert(((low == 0ULL) && (high == 0ULL)) && "map() failed (1)");
    internals::inputMap::map(true, rawBitmap, low, high);
    assert(((low == 0ULL) && (high == 0ULL)) && "map() failed (2)");

    rawBitmap = 0b001;
    internals::inputMap::map(false, rawBitmap, low, high);
    assert(((low == 0ULL) && (high == BMPH(64))) && "map() failed (3)");
    internals::inputMap::map(true, rawBitmap, low, high);
    assert(((low == 0ULL) && (high == BMPH(127))) && "map() failed (4)");

    rawBitmap = 0b011;
    internals::inputMap::map(false, rawBitmap, low, high);
    assert(((low == 0ULL) && (high == BMPH(64))) && "map() failed (5)");
    internals::inputMap::map(true, rawBitmap, low, high);
    assert(((low == 0ULL) && (high == BMPH(127))) && "map() failed (6)");

    rawBitmap = 0b100;
    internals::inputMap::map(false, rawBitmap, low, high);
    assert(((low == BMPL(0)) && (high == 0ULL)) && "map() failed (7)");
    internals::inputMap::map(true, rawBitmap, low, high);
    assert(((low == BMPL(1)) && (high == 0ULL)) && "map() failed (8)");

    rawBitmap = 0b111;
    internals::inputMap::map(false, rawBitmap, low, high);
    assert(((low == BMPL(0)) && (high == BMPH(64))) && "map() failed (9)");
    internals::inputMap::map(true, rawBitmap, low, high);
    assert(((low == BMPL(1)) && (high == BMPH(127))) && "map() failed (10)");

    rawBitmap = 0b1000;
    internals::inputMap::map(false, rawBitmap, low, high);
    assert(((low == rawBitmap) && (high == 0ULL)) && "map() failed (11)");
    internals::inputMap::map(true, rawBitmap, low, high);
    assert(((low == 0ULL) && (high == rawBitmap)) && "map() failed (12)");
}

void test5()
{
    std::cout << "- Test 5 -" << std::endl;
    uint8_t alt, noAlt;
    uint64_t rawBitmap, low, high;

    reset();
    // Custom defaults
    inputMap::set(1, 32, 33);
    // start
    internals::inputMap::getReady();
    OnStart::notify();

    // Override custom defaults
    InputMapService::call::setMap(1, 0, 0);

    // Restore custom defaults
    InputMapService::call::resetMap();
    InputMapService::call::getMap(1, noAlt, alt);
    assert((noAlt == 32) && "Default map not restored (1)");
    assert((alt == 33) && "Default map not restored (2)");
    InputMapService::call::getMap(0, noAlt, alt);
    assert((noAlt == 0) && "Default map not restored (3)");
    assert((alt == 64) && "Default map not restored (4)");
}

void test6()
{
    std::cout << "- Test 6 -" << std::endl;

    // Set specific input numbers for this test
    reset();
    InputNumber::clearBook();
    InputNumber n;
    n = 0;
    n.book();
    n = 2;
    n.book();
    n = 3;
    n.book();

    // start
    inputMap::setOptimal();
    inputMap::set(3, 20, 20);
    internals::inputMap::getReady();
    OnStart::notify();

    // Test
    uint8_t alt, noAlt;
    InputMapService::call::getMap(0, noAlt, alt);
    assert((noAlt == 0) && "Optimal not set (1)");
    assert((alt == 4) && "Optimal not set (2)");

    InputMapService::call::getMap(2, noAlt, alt);
    assert((noAlt == 2) && "Optimal not set (3)");
    assert((alt == 6) && "Optimal not set (4)");

    InputMapService::call::getMap(3, noAlt, alt);
    assert((noAlt == 20) && "Optimal did not respect user setting (1)");
    assert((alt == 20) && "Optimal did not respect user setting (2)");

    // Restore input numbers
    InputNumber::bookAll();
}

int main()
{
    LoadSetting::subscribe(loadSettingsCallback);

    InputNumber::bookAll();

    // Basic parameter test
    try
    {
        inputMap::set(77, 0, 0);
        assert(false && "Invalid input number was successfully mapped (1)");
    }
    catch (std::runtime_error)
    {
    }

    try
    {
        inputMap::set(0, 200, 0);
        assert(false && "Invalid input number was successfully mapped (2)");
    }
    catch (std::runtime_error)
    {
    }

    try
    {
        inputMap::set(0, 254);
        assert(false && "Invalid input number was successfully mapped (3)");
    }
    catch (std::runtime_error)
    {
    }

    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
}