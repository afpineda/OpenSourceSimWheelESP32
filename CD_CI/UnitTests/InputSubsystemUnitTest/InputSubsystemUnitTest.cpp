/**
 * @file InputSubsystemUnitTest.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-10
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
#include "InternalTypes.hpp"
#include "InternalServices.hpp"
#include "cd_ci_assertions.hpp"
#include <iostream>
#include <semaphore>
#include <chrono>

using namespace std;

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

FakeInput *primary, *secondary;

DecouplingEvent receivedEvent;
std::counting_semaphore<1> received{1};

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

void waitFor(std::string message = "")
{
    if (!received.try_acquire_for(std::chrono::milliseconds(300)))
    {
        std::cout << "Input event not received at: " << message << std::endl;
        assert(false && "Input event not received");
    }
}

void reset()
{
    primary->state = {};
    primary->leftAxis = 0;
    primary->rightAxis = 0;
    secondary->state = {};
    if ((receivedEvent.leftAxisValue != 0) ||
        (receivedEvent.rightAxisValue != 0) ||
        (receivedEvent.rawInputBitmap))
        waitFor("reset");
}

//------------------------------------------------------------------
// Mock
//------------------------------------------------------------------

void internals::inputHub::onRawInput(DecouplingEvent &event)
{
    receivedEvent = event;
    // std::cout << "Rec: " << receivedEvent.rawInputBitmap << std::endl;
    received.release();
}

//------------------------------------------------------------------
//------------------------------------------------------------------
// Test groups
//------------------------------------------------------------------
//------------------------------------------------------------------

/**
 * @brief Check that update() forces a new input event
 *
 */
void test1()
{
    std::cout << "- test 1 -" << std::endl;
    InputService::call::update();
    waitFor("1");
    binEquals("update (bitmap)", 0, receivedEvent.rawInputBitmap.low);
}

/**
 * @brief Check that a new input bitmap is detected
 *
 */
void test2()
{
    std::cout << "- test 2 -" << std::endl;
    reset();
    primary->state.low = 0b011;
    waitFor("1");
    binEquals("single input (bitmap)", 0b011, receivedEvent.rawInputBitmap.low);
}

/**
 * @brief Check combined input from two hardware devices
 *
 */
void test3()
{
    std::cout << "- test 3 -" << std::endl;
    reset();

    primary->state.low = 0b0011;
    waitFor("1");
    binEquals("1st input (bitmap)", 0b011, receivedEvent.rawInputBitmap.low);
    secondary->state.low = 0b1100;
    waitFor("2");
    binEquals("2nd input (bitmap)", 0b1111, receivedEvent.rawInputBitmap.low);
}

/**
 * @brief Check events caused by buttons being pressed and released
 *
 */
void test4()
{
    std::cout << "- test 4 -" << std::endl;
    reset();
    primary->press(0);
    waitFor("1");
    binEquals("1 input (bitmap)", 0b0001, receivedEvent.rawInputBitmap.low);
    secondary->press(2);
    waitFor("2");
    binEquals("2 input (bitmap)", 0b0101, receivedEvent.rawInputBitmap.low);
    primary->release(0);
    waitFor("3");
    binEquals("3 input (bitmap)", 0b0100, receivedEvent.rawInputBitmap.low);
    secondary->release(2);
    waitFor("4");
    binEquals("4 input (bitmap)", 0b0000, receivedEvent.rawInputBitmap.low);
}

/**
 * @brief Check events caused by axes being moved
 *
 */
void test5()
{
    std::cout << "- test 5 -" << std::endl;
    reset();
    primary->leftAxis = 33;
    primary->rightAxis = 66;
    waitFor("1");
    assert<int>::equals("1 axis L", 33, receivedEvent.leftAxisValue);
    assert<int>::equals("1 axis R", 66, receivedEvent.rightAxisValue);
    primary->leftAxis = 66;
    waitFor("2");
    assert<int>::equals("2 axis L", 66, receivedEvent.leftAxisValue);
    assert<int>::equals("2 axis R", 66, receivedEvent.rightAxisValue);
    primary->rightAxis = 22;
    waitFor("3");
    assert<int>::equals("3 axis L", 66, receivedEvent.leftAxisValue);
    assert<int>::equals("3 axis R", 22, receivedEvent.rightAxisValue);
}

/**
 * @brief Check polarity reversion
 *
 */
void test6()
{
    std::cout << "- test 6 -" << std::endl;
    reset();
    InputService::call::setAxisPolarity(false, false, false);

    primary->leftAxis = 54;
    primary->rightAxis = 1;
    waitFor("1");
    assert<int>::equals("L axis", 54, receivedEvent.leftAxisValue);
    assert<int>::equals("R axis", 1, receivedEvent.rightAxisValue);

    InputService::call::reverseLeftAxis();
    waitFor("2");
    assert<int>::equals(
        "L axis reverse failed", 254 - 54, receivedEvent.leftAxisValue);

    InputService::call::reverseRightAxis();
    waitFor("3");
    assert<int>::equals(
        "R axis reverse failed", 254 - 1, receivedEvent.rightAxisValue);
}

/**
 * @brief Check axis recalibration
 *
 */
void test7()
{
    std::cout << "- test 7 -" << std::endl;
    reset();
    assert<size_t>::equals(
        "initial state", 0, primary->recalibrationRequestCount);

    InputService::call::recalibrateAxes();
    assert<size_t>::equals(
        "recalibration", 2, primary->recalibrationRequestCount);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
// Entry point
//------------------------------------------------------------------
//------------------------------------------------------------------

int main()
{
    primary = new FakeInput();
    primary->mask.low = ~(0b0011);
    primary->mask.high = ~0ULL;
    secondary = new FakeInput();
    secondary->mask.low = ~(0b1100);
    secondary->mask.high = ~0ULL;
    internals::inputs::addFakeInput(primary);
    internals::inputs::addFakeInput(secondary);
    internals::inputs::getReady();
    OnStart();
    waitFor();

    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();

    return 0;
}
