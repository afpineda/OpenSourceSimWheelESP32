/**
 * @file UInt128Test.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2026-03-14
 * @brief Unit test
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheelTypes.hpp"
#include <iostream>
#include <utility>
#include <cassert>

using namespace std;

uint128_t num1{
    .low = 0ULL,
    .high = 0x8000000000000001,
};

uint128_t num2{
    .low = 0x8000000000000001,
    .high = 0ULL,
};

uint128_t num3{
    .low = 0x8000000000000001,
    .high = 0x8000000000000001,
};

void test0()
{
    cout << "- Initialization - " << endl;
    {
        uint128_t n{};
        assert(n.low == 0ULL);
        assert(n.high == 0ULL);
    }
    {
        uint128_t n{num1};
        assert(n.low == num1.low);
        assert(n.high == num1.high);
    }
    {
        uint128_t n;
        n = num1;
        assert(n.low == num1.low);
        assert(n.high == num1.high);
    }
    {
        uint128_t aux{num1};
        uint128_t n{::std::move(aux)};
        assert(n.low == num1.low);
        assert(n.high == num1.high);
    }
    {
        uint128_t aux{num1};
        uint128_t n;
        n = ::std::move(aux);
        assert(n.low == num1.low);
        assert(n.high == num1.high);
    }
}

void test1()
{
    cout << "- (in)Equality - " << endl;
    assert(num1 == num1);
    assert(num2 == num2);
    assert(num3 == num3);

    assert(num1 != num2);
    assert(num1 != num3);
    assert(num2 != num1);
    assert(num2 != num1);

    assert(num2 != num3);
    assert(num3 != num2);
}

void test2()
{
    cout << "- Bit test - " << endl;
    {
        uint128_t num4{};
        for (uint8_t i = 0; i < 255; i++)
            assert(!num4.bit(i));
    }
    {
        uint128_t num4 = ~uint128_t{};
        for (uint8_t i = 0; i < 128; i++)
            assert(num4.bit(i));
    }
    assert(!num1.bit(0));
    assert(!num1.bit(63));
    assert(num1.bit(64));
    assert(!num1.bit(65));
    assert(!num1.bit(126));
    assert(num1.bit(127));
    assert(!num1.bit(128));
}

void test3()
{
    cout << "- Bit set/clear - " << endl;
    {
        uint128_t num4 = num1;
        num4.set_bit(64, false);
        num4.set_bit(127, false);
        assert(num4 == uint128_t{});
    }
    {
        uint128_t num4 = num2;
        num4.set_bit(0, false);
        num4.set_bit(63, false);
        assert(num4 == uint128_t{});
    }
    {
        uint128_t num4{};
        num4.set_bit(0);
        num4.set_bit(63);
        num4.set_bit(64);
        num4.set_bit(127);
        assert(num4 == num3);
    }
}

void test4()
{
    cout << "- Bitwise operators - " << endl;

    // OR
    {
        uint128_t num4 = num1 | num1;
        assert(num4 == num1);
    }
    {
        uint128_t num4 = num1 | num2;
        assert(num4 == num3);
    }

    // AND
    {
        uint128_t num4 = num1 & num1;
        assert(num4 == num1);
    }
    {
        uint128_t num4 = num1 & num2;
        assert(num4 == uint128_t{});
    }
    {
        uint128_t num4{
            .low = 0ULL,
            .high = ~0ULL,
        };
        assert((num4 & num1) == num1);
        assert((num4 & num2) == uint128_t{});
    }

    // XOR
    {
        uint128_t num4 = num2 ^ num2;
        assert(num4 == uint128_t{});
    }
    {
        uint128_t num4 = num1 ^ num2;
        assert(num4 == num3);
    }
    {
        uint128_t a{
            .low = 0b1,
            .high = 0b1,
        };
        uint128_t b{
            .low = 0b10,
            .high = 0b10,
        };
        uint128_t r{
            .low = 0b11,
            .high = 0b11,
        };
        assert((a ^ b) == r);
    }

    // NOT
    {
        uint128_t h{
            .low = 0xFFFFFFFFFFFFFFFF,
            .high = 0xFFFFFFFFFFFFFFFF,
        };
        uint128_t z{};
        assert(~h == z);
        assert(~z == h);
    }

    // Shift right
    {
        uint128_t expected{
            .low = 0xC000000000000000,
            .high = 0x4000000000000000,
        };
        uint128_t result = (num3 >> 1);
        assert(expected == result);
    }
    assert((num1 >> 64) == num2);

    // Shift left
    {
        uint128_t expected{
            .low = 0x02,
            .high = 0x03,
        };
        uint128_t result = (num3 << 1);
        assert(expected == result);
    }
    assert((num2 << 64) == num1);
}

void test5()
{
    cout << "- Bitmap -" << endl;
    {
        uint128_t t = uint128_t::bitmap(0);
        uint128_t expected{
            .low = 1,
            .high = 0ULL,
        };
        assert(t == expected);
    }
    {
        uint128_t t = uint128_t::bitmap(63);
        uint128_t expected{
            .low = 0x8000000000000000,
            .high = 0ULL,
        };
        assert(t == expected);
    }
    {
        uint128_t t = uint128_t::bitmap(64);
        uint128_t expected{
            .low = 0ULL,
            .high = 1ULL,
        };
        assert(t == expected);
    }
    {
        uint128_t t = uint128_t::bitmap(127);
        uint128_t expected{
            .low = 0ULL,
            .high = 0x8000000000000000,
        };
        assert(t == expected);
    }
    {
        uint128_t t = uint128_t::bitmap(128);
        uint128_t expected{};
        assert(t == expected);
    }
}

void test6()
{
    cout << "- Compound bitwise operators - " << endl;

    // OR
    {
        uint128_t num4 = num1;
        num4 |= num1;
        assert(num4 == num1);
    }
    {
        uint128_t num4 = num1;
        num4 |= num2;
        assert(num4 == num3);
    }

    // AND
    {
        uint128_t num4 = num1;
        num4 &= num1;
        assert(num4 == num1);
    }
    {
        uint128_t num4 = num1;
        num4 &= num2;
        assert(num4 == uint128_t{});
    }
    {
        uint128_t num4{
            .low = 0ULL,
            .high = ~0ULL,
        };
        uint128_t num5 = num4;
        num4 &= num1;
        num5 &= num2;
        assert(num4 == num1);
        assert(num5 == uint128_t{});
    }

    // XOR
    {
        uint128_t num4 = num2;
        num4 ^= num2;
        assert(num4 == uint128_t{});
    }
    {
        uint128_t num4 = num1;
        num4 ^= num2;
        assert(num4 == num3);
    }
    {
        uint128_t a{
            .low = 0b1,
            .high = 0b1,
        };
        uint128_t b{
            .low = 0b10,
            .high = 0b10,
        };
        uint128_t r{
            .low = 0b11,
            .high = 0b11,
        };
        uint128_t num4 = a;
        num4 ^= b;
        assert(num4 == r);
    }
}

void test7()
{
    cout << "- Boolean typecast - " << endl;
    {
        uint128_t zero{};
        assert(!zero);
    }
    assert(num1);
    assert(num2);
    assert(num3);
    assert(uint128_t::neg());
}

int main()
{
    test0();
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    return 0;
}