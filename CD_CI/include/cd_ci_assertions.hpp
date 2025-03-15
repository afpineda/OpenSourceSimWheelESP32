/**
 * @file cd_ci_assertion.hpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-21
 * @brief Utility for testing
 *
 * @copyright Licensed under the EUPL
 *
 */

//------------------------------------------------------------------
// Imports
//------------------------------------------------------------------

#include <iostream>
#include <bitset>
#include <cassert>

//------------------------------------------------------------------
// Assertions
//------------------------------------------------------------------

template <typename T>
struct assert
{
    static void equals(std::string message, T expected, T found)
    {
        if (expected != found)
        {
            std::cout << "[assert.equals] Expected: " << expected;
            std::cout << " Found: " << found << std::endl;
            std::cout << " At: " << message << std::endl;
            assert(false && "assertion failed");
        }
    }

    static void nonEquals(std::string message, T expected, T found)
    {
        if (expected == found)
        {
            std::cout << "[assert.nonEquals] Expected: " << expected;
            std::cout << " Found: " << found << std::endl;
            std::cout << " At: " << message << std::endl;
            assert(false && "assertion failed");
        }
    }

    static void almostEquals(std::string message, T expected, T found, T tolerance)
    {
        T lowerLimit = expected - tolerance;
        T upperLimit = expected + tolerance;
        if ((found < lowerLimit) || (found > upperLimit))
        {
            std::cout << "[assert.almostEquals] Expected: " << expected;
            std::cout << " Found: " << found << std::endl;
            std::cout << " At: " << message << std::endl;
            assert(false && "assertion failed");
        }
    }

    static void less(std::string message, T expected, T found)
    {
        if (expected <= found)
        {
            std::cout << "[assert.less] Expected: " << expected;
            std::cout << " Found: " << found << std::endl;
            std::cout << " At: " << message << std::endl;
            assert(false && "assertion failed");
        }
    }

    static void more(std::string message, T expected, T found)
    {
        if (expected >= found)
        {
            std::cout << "[assert.more] Expected: " << expected;
            std::cout << " Found: " << found << std::endl;
            std::cout << " At: " << message << std::endl;
            assert(false && "assertion failed");
        }
    }

    // static void lessOrEqual(std::string message, T expected, T found)
    // {
    //     if (expected < found)
    //     {
    //         std::cout << "[assert.lessOrEqual] Expected: " << expected;
    //         std::cout << " Found: " << found << std::endl;
    //         std::cout << " At: " << message << std::endl;
    //         assert(false && "assertion failed");
    //     }
    // }
};

//------------------------------------------------------------------

static void binEquals(std::string message, uint64_t expected, uint64_t found)
{
    if (expected != found)
    {
        std::cout << "mismatch at: " << message << std::endl;
        std::cout << "Expected: " << std::bitset<64>(expected) << std::endl;
        std::cout << "Found: " << std::bitset<64>(found) << std::endl;
        assert(false && "unexpected input");
    }
}

static void byteEquals(std::string message, size_t size, const uint8_t *expected, const uint8_t *found)
{
    assert((expected != nullptr) && "expected data is null");
    assert((found != nullptr) && "expected data is null");
    for (size_t i = 0; i < size; i++)
        if (expected[i] != found[i])
        {
            std::cout << "Mismatch at: " << message << std::endl;
            std::cout << "Index: " << i << std::endl;
            std::cout << "Expected: " << (int)expected[i] << std::endl;
            std::cout << "Found: " << (int)found[i] << std::endl;
            assert(false && "unexpected input");
        }
}