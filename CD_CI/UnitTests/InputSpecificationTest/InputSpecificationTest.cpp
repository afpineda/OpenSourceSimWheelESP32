/**
 * @file InputSpecificationTest.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-04
 * @brief Unit test
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "InputSpecification.hpp"
#include <cassert>
#include <iostream>

void BtnMtxTest()
{
    ButtonMatrix mtx;
    mtx[0][1] = 1;
    mtx[0][2] = 2;
    mtx[1][1] = 3;
    mtx[1][3] = 5;
}

void MuxTest()
{
    InputNumber::clearBook();
    GPIO::clearReservations();
    AnalogMultiplexerChip8 chip1(0), chip2(1);
    assert(((uint8_t)chip1.getInputGPIO() == 0) && "Mux: Failed initialization (1)");
    assert(((uint8_t)chip2.getInputGPIO() == 1) && "Mux: Failed initialization (2)");
    chip1[Mux8Pin::A0] = 0;
    chip1[Mux8Pin::A7] = 7;
    chip2[Mux8Pin::A0] = 1;
    chip2[Mux8Pin::A1] = 6;
    chip1.reserve_and_book();
    chip2.reserve_and_book();
    assert((InputNumber::booked() == 0b11000011) && "Mux: wrong booked inputs");
    try
    {
        GPIO n(0);
        n.reserve();
        assert(false && "Mux: GPIO reservation failed");
    }
    catch (gpio_error)
    {
    }
}

void ExpanderTest()
{
    PCF8574Expander exp;
    exp[PCF8574Pin::P0] = 0;
    exp[PCF8574Pin::P7] = 7;
}

void ShiftRegisterTest()
{
    const auto testPin = SR8Pin::A;
    ShiftRegisterChip thirdSR;
    ShiftRegisterChip secondSR;
    ShiftRegisterChip firstSR;
    firstSR[testPin] = 1;
    secondSR[testPin] = 2;
    thirdSR[testPin] = 3;

    ShiftRegisterChain chain = { firstSR, secondSR, thirdSR };
}

int main()
{
    BtnMtxTest();
    MuxTest();
    ExpanderTest();
    ShiftRegisterTest();
}