/**
 * @file BitQueueTest.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-04
 * @brief Unit test
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "InternalTypes.hpp"
#include <cassert>
#include <iostream>
#include <array>

typedef std::array<bool, 32> TestSequence;
#define T true
#define F false

TestSequence seq1 = {T, F, F, T,
                     F, F, T, F,
                     T, T, F, T,
                     T, F, F, F,
                     F, F, T, F,
                     F, F, F, F,
                     F, F, F, F,
                     F, F, T, T};

TestSequence seq2 = {T, T, F, F,
                     T, T, F, F,
                     T, T, F, T,
                     F, T, F, T,
                     F, T, F, T,
                     F, F, F, F,
                     T, T, T, T,
                     F, F, F, F};

bool sequence_eq(TestSequence &expected, TestSequence &actual, size_t max = 32, size_t startIndex = 0)
{
    for (size_t i = startIndex; ((i < max) && i < 32); i++)
        if (expected[i] != actual[i])
            return false;
    return true;
}

size_t pop_sequence(BitQueue &q, TestSequence &result, size_t max = 32)
{
    size_t count = 0;
    bool item;
    while ((count < max) && (count < 32) && q.dequeue(item))
        result[count++] = item;
    return count;
}

void push_sequence(BitQueue &q, TestSequence &from, size_t max = 32)
{
    for (size_t i = 0; ((i < max) && i < 32); i++)
    {
        // std::cout << "push " << from[i] << std::endl;
        q.enqueue(from[i]);
    }
}

void test0()
{
    std::cout << "- Test 0-" << std::endl;
    uint8_t v = 0;
    for (int i = 0; i < 63; i++)
        BitQueue::incDataPointer(v);
    assert((v == 63) && "incBitQueuePointer not working (1)");
    BitQueue::incDataPointer(v);
    BitQueue::incDataPointer(v);
    assert((v == 1) && "incBitQueuePointer not working (2)");
}

void test1()
{
    std::cout << "- Test 1-" << std::endl;
    BitQueue q;
    bool v, result;
    assert((!q.dequeue(v)) && "Queue is not empty at creation");
    q.enqueue(true);
    result = q.dequeue(v);
    assert(result && "Queue is empty after push (1)");
    assert(v && "push(true) does not correspond to pop(true)");
    q.enqueue(false);
    result = q.dequeue(v);
    assert(result && "Queue is empty after push (2)");
    assert(!v && "push(false) does not correspond to pop(false)");
    assert((!q.dequeue(v)) && "Queue is not empty after pop");
}

void test2()
{
    std::cout << "- Test 2-" << std::endl;
    bool result;
    size_t count;
    TestSequence actual;
    result = sequence_eq(seq1, seq1);
    assert(result && "Self-test failed in sequence_eq()");

    BitQueue q1;
    push_sequence(q1, seq1);
    count = pop_sequence(q1, actual);
    assert((count == 32) && "Seq1 not fully pushed/poped");
    result = sequence_eq(seq1, actual);
    assert(result && "Seq1 is not equal to the poped sequence");

    push_sequence(q1, seq1);
    push_sequence(q1, seq1);
    pop_sequence(q1, actual);
    result = sequence_eq(seq1, actual);
    assert(result && "Seq1+seq1 is not equal to the poped sequence (1)");
    count = pop_sequence(q1, actual);
    assert((count == 31) && "Seq1 not fully pushed/poped");
    result = sequence_eq(seq1, actual, 31);
    assert(result && "Seq1+seq1 is not equal to the poped sequence (2)");
}

int main()
{
    test0();
    test1();
    test2();
}
