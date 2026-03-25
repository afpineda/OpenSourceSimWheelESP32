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
#include <cassert>

bool loaded = false;

void reset()
{
    InputMapService::reset();
    OnStart.clear();
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
    OnStart();

    assert((loaded) && "user map not loaded from storage");
    InputMapService::call().getMap(9, noAlt, alt);
    assert((noAlt == 9) && "default map not set (1)");
    assert((alt == 9 + 64) && "default map not set (2)");
    InputMapService::call().getMap(63, noAlt, alt);
    assert((noAlt == 63) && "default map not set (3)");
    assert((alt == 63 + 64) && "default map not set (4)");
    InputMapService::call().getMap(64, noAlt, alt);
    assert((noAlt == 64) && "default map not set (5)");
    assert((alt == 0) && "default map not set (6)");
    InputMapService::call().getMap(127, noAlt, alt);
    assert((noAlt == 127) && "default map not set (7)");
    assert((alt == 63) && "default map not set (8)");
}

void test2()
{
    std::cout << "- Test 2 -" << std::endl;
    uint8_t alt, noAlt;

    reset();
    inputMap::set(0, 1, 2);
    inputMap::set(64, 1, 2);
    internals::inputMap::getReady();
    OnStart();

    InputMapService::call().getMap(0, noAlt, alt);
    assert((noAlt == 1) && "custom firmware-defined map not respected (1)");
    assert((alt == 2) && "custom firmware-defined map not respected (2)");

    InputMapService::call().getMap(7, noAlt, alt);
    assert((noAlt == 7) && "default map not set (1)");
    assert((alt == 7 + 64) && "default map not set (2)");

    InputMapService::call().getMap(64, noAlt, alt);
    assert((noAlt == 1) && "custom firmware-defined map not respected (3)");
    assert((alt == 2) && "custom firmware-defined map not respected (4)");

    InputMapService::call().getMap(65, noAlt, alt);
    assert((noAlt == 65) && "default map not set (3)");
    assert((alt == 1) && "default map not set (4)");
}

void test3()
{
    std::cout << "- Test 3 -" << std::endl;
    uint8_t alt, noAlt;

    reset();
    internals::inputMap::getReady();
    OnStart();

    alt = noAlt = 0;
    InputMapService::call().setMap(200, 0, 1); // no exception should arise
    InputMapService::call().setMap(0, 128, 0);
    InputMapService::call().getMap(0, noAlt, alt);
    assert((noAlt == 0) && "setMap failed (1)");
    assert((alt == 64) && "setMap failed (2)");

    alt = noAlt = 0;
    InputMapService::call().setMap(0, 0, 220);
    InputMapService::call().getMap(0, noAlt, alt);
    assert((noAlt == 0) && "setMap failed (3)");
    assert((alt == 64) && "setMap failed (4)");

    alt = noAlt = 0;
    InputMapService::call().setMap(0, 63, 127);
    InputMapService::call().getMap(0, noAlt, alt);
    assert((noAlt == 63) && "setMap failed (5)");
    assert((alt == 127) && "setMap failed (6)");

    alt = noAlt = 0;
    InputMapService::call().setMap(127, 63, 127);
    InputMapService::call().getMap(127, noAlt, alt);
    assert((noAlt == 63) && "setMap failed (7)");
    assert((alt == 127) && "setMap failed (8)");
}

#define BMPL(n) (1ULL << (n))
#define BMPH(n) (1ULL << (n - 64))

void test4()
{
    std::cout << "- Test 4 -" << std::endl;
    uint8_t alt, noAlt;

    reset();
    inputMap::set(0, 64, 127);
    inputMap::set(1, 64, 127);
    inputMap::set(2, 0, 1);
    internals::inputMap::getReady();
    OnStart();

    {
        uint128_t rawBitmap{};
        internals::inputMap::map(false, rawBitmap);
        assert(((rawBitmap.low == 0ULL) && (rawBitmap.high == 0ULL)) &&
               "map() failed (1)");
    }
    {
        uint128_t rawBitmap{};
        internals::inputMap::map(true, rawBitmap);
        assert(((rawBitmap.low == 0ULL) && (rawBitmap.high == 0ULL)) &&
               "map() failed (2)");
    }
    {
        uint128_t rawBitmap{
            .low = 0b001,
            .high = 0ULL,
        };
        internals::inputMap::map(false, rawBitmap);
        assert(((rawBitmap.low == 0ULL) && (rawBitmap.high == BMPH(64))) &&
               "map() failed (3)");
    }
    {
        uint128_t rawBitmap{
            .low = 0b001,
            .high = 0ULL,
        };
        internals::inputMap::map(true, rawBitmap);
        assert(((rawBitmap.low == 0ULL) && (rawBitmap.high == BMPH(127))) &&
               "map() failed (4)");
    }
    {
        uint128_t rawBitmap{
            .low = 0b011,
            .high = 0ULL,
        };
        internals::inputMap::map(false, rawBitmap);
        assert(((rawBitmap.low == 0ULL) && (rawBitmap.high == BMPH(64))) &&
               "map() failed (5)");
    }
    {
        uint128_t rawBitmap{
            .low = 0b011,
            .high = 0ULL,
        };
        internals::inputMap::map(true, rawBitmap);
        assert(((rawBitmap.low == 0ULL) && (rawBitmap.high == BMPH(127))) &&
               "map() failed (6)");
    }
    {
        uint128_t rawBitmap{
            .low = 0b100,
            .high = 0ULL,
        };
        internals::inputMap::map(false, rawBitmap);
        assert(((rawBitmap.low == BMPL(0)) && (rawBitmap.high == 0ULL)) &&
               "map() failed (7)");
    }
    {
        uint128_t rawBitmap{
            .low = 0b100,
            .high = 0ULL,
        };
        internals::inputMap::map(true, rawBitmap);
        assert(((rawBitmap.low == BMPL(1)) && (rawBitmap.high == 0ULL)) &&
               "map() failed (8)");
    }
    {
        uint128_t rawBitmap{
            .low = 0b111,
            .high = 0ULL,
        };
        internals::inputMap::map(false, rawBitmap);
        assert(((rawBitmap.low == BMPL(0)) && (rawBitmap.high == BMPH(64))) &&
               "map() failed (9)");
    }
    {
        uint128_t rawBitmap{
            .low = 0b111,
            .high = 0ULL,
        };
        internals::inputMap::map(true, rawBitmap);
        assert(((rawBitmap.low == BMPL(1)) && (rawBitmap.high == BMPH(127))) &&
               "map() failed (10)");
    }
    {
        uint128_t rawBitmap{
            .low = 0b1000,
            .high = 0ULL,
        };
        internals::inputMap::map(false, rawBitmap);
        assert(((rawBitmap.low == 0b1000) && (rawBitmap.high == 0ULL)) &&
               "map() failed (11)");
    }
    {
        uint128_t rawBitmap{
            .low = 0b1000,
            .high = 0ULL,
        };
        internals::inputMap::map(true, rawBitmap);
        assert(((rawBitmap.low == 0ULL) && (rawBitmap.high == 0b1000)) &&
               "map() failed (12)");
    }
}

void test5()
{
    std::cout << "- Test 5 -" << std::endl;
    uint8_t alt, noAlt;

    reset();
    // Custom defaults
    inputMap::set(1, 32, 33);
    // start
    internals::inputMap::getReady();
    OnStart();

    // Override custom defaults
    InputMapService::call().setMap(1, 0, 0);

    // Restore custom defaults
    InputMapService::call().resetMap();
    InputMapService::call().getMap(1, noAlt, alt);
    assert((noAlt == 32) && "Default map not restored (1)");
    assert((alt == 33) && "Default map not restored (2)");
    InputMapService::call().getMap(0, noAlt, alt);
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
    OnStart();

    // Test
    uint8_t alt, noAlt;
    InputMapService::call().getMap(0, noAlt, alt);
    assert((noAlt == 0) && "Optimal not set (1)");
    assert((alt == 4) && "Optimal not set (2)");

    InputMapService::call().getMap(2, noAlt, alt);
    assert((noAlt == 2) && "Optimal not set (3)");
    assert((alt == 6) && "Optimal not set (4)");

    InputMapService::call().getMap(3, noAlt, alt);
    assert((noAlt == 20) && "Optimal did not respect user setting (1)");
    assert((alt == 20) && "Optimal did not respect user setting (2)");

    // Restore input numbers
    InputNumber::bookAll();
}

void test7()
{
    std::cout << "- Test 7 -" << std::endl;

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
    inputMap::set(1, 20, 20);
    internals::inputMap::getReady();
    try
    {
        OnStart();
        assert(false && "Non-existing input number was successfully mapped");
    }
    catch (std::runtime_error)
    {
    }
}

int main()
{
    LoadSetting.subscribe(loadSettingsCallback);

    InputNumber::bookAll();

    // Basic parameter test
    try
    {
        inputMap::set(200, 0, 0);
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

    try
    {
        inputMap::set(UNSPECIFIED::VALUE, 0, 0);
        assert(false && "Invalid input number was successfully mapped (4)");
    }
    catch (std::runtime_error)
    {
    }

    try
    {
        inputMap::set(0, UNSPECIFIED::VALUE, 0);
        assert(false && "Invalid input number was successfully mapped (5)");
    }
    catch (std::runtime_error)
    {
    }

    try
    {
        inputMap::set(0, 0, UNSPECIFIED::VALUE);
        assert(false && "Invalid input number was successfully mapped (6)");
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
    test7();

    return 0;
}