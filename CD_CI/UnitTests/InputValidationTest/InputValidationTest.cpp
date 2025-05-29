/**
 * @file InputValidationTest.cpp
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-02-04
 * @brief Unit test
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "InputValidation.hpp"
#include <cassert>
#include <iostream>
#include <string>

using namespace internals::inputs;

void checkReservation(int gpioNum, std::string errorMsg)
{
    try
    {
        GPIO aux(gpioNum);
        aux.reserve();
        throw std::runtime_error(errorMsg);
    }
    catch (gpio_error)
    {
        // OK
    }
}

void BtnMtxTest()
{
    std::cout << "- Button matrix -" << std::endl;
    GPIO::clearReservations();
    InputNumber::clearBook();

    ButtonMatrix mtx;
    mtx[0][10] = 1;
    mtx[0][20] = 2;
    mtx[1][10] = 4;
    mtx[1][30] = 6;

    validate::buttonMatrix(mtx);
    checkReservation(0, "Failed reservation GPIO 0");
    checkReservation(1, "Failed reservation GPIO 1");
    checkReservation(10, "Failed reservation GPIO 10");
    checkReservation(20, "Failed reservation GPIO 20");
    checkReservation(30, "Failed reservation GPIO 30");
    assert((InputNumber::booked() == 0b1010110) && "Wrong booked I.N.");

    GPIO::clearReservations();
    InputNumber::clearBook();
    try
    {
        ButtonMatrix empty;
        validate::buttonMatrix(empty);
        assert(false && "Empty button matrix accepted");
    }
    catch (empty_input_number_set)
    {
    }
}

void MuxTest()
{
    std::cout << "- Multiplexers -" << std::endl;
    GPIO::clearReservations();
    InputNumber::clearBook();

    AnalogMultiplexerChip8 chip1(10);
    chip1[Mux8Pin::A0] = 1;
    chip1[Mux8Pin::A7] = 2;
    AnalogMultiplexerChip8 chip2(20);
    chip2[Mux8Pin::A0] = 3;
    chip2[Mux8Pin::A7] = 4;

    validate::analogMultiplexer<Mux8Pin>({1, 3}, {chip1, chip2});
    checkReservation(1, "Failed reservation GPIO 1");
    checkReservation(3, "Failed reservation GPIO 3");
    checkReservation(10, "Failed reservation GPIO 10");
    checkReservation(20, "Failed reservation GPIO 20");
    assert((InputNumber::booked() == 0b11110) && "Wrong booked I.N.");

    GPIO::clearReservations();
    InputNumber::clearBook();
    try
    {
        AnalogMultiplexerChip8 empty(10);
        validate::analogMultiplexer<Mux8Pin>({1, 3}, {empty});
        assert(false && "Empty analog multiplexer group accepted");
    }
    catch (empty_input_number_set)
    {
    }

    try
    {
        validate::analogMultiplexer<Mux8Pin>({}, {});
        assert(false && "Trivial analog multiplexer group accepted");
    }
    catch (empty_input_number_set)
    {
    }
}

void ShiftRegisterTest()
{
    std::cout << "- Shift registers -" << std::endl;
    GPIO::clearReservations();
    InputNumber::clearBook();

    ShiftRegisterChip chip1;
    chip1[SR8Pin::A] = 0;
    chip1[SR8Pin::H] = 2;
    ShiftRegisterChip chip2;
    chip2[SR8Pin::A] = 1;
    chip2[SR8Pin::H] = 2;

    validate::shiftRegisterChain(0, 1, 2, {chip1, chip2});
    checkReservation(0, "Failed reservation GPIO 0");
    checkReservation(1, "Failed reservation GPIO 1");
    checkReservation(2, "Failed reservation GPIO 2");
    assert((InputNumber::booked() == 0b111) && "Wrong booked I.N.");

    GPIO::clearReservations();
    InputNumber::clearBook();
    try
    {
        ShiftRegisterChip empty;
        validate::shiftRegisterChain(0, 1, 2, {empty});
        assert(false && "Empty shift register chain accepted");
    }
    catch (empty_input_number_set)
    {
    }

    try
    {
        validate::shiftRegisterChain(3, 4, 5, {});
        assert(false && "Trivial shift register chain accepted");
    }
    catch (empty_input_number_set)
    {
    }
}

void GPIOExpanderTest()
{
    std::cout << "- GPIO Expander -" << std::endl;
    GPIO::clearReservations();
    InputNumber::clearBook();

    PCF8574Expander chip1;
    chip1[PCF8574Pin::P0] = 0;
    chip1[PCF8574Pin::P6] = 6;
    validate::GPIOExpander<PCF8574Pin>(chip1);
    assert((InputNumber::booked() == 0b1000001) && "Wrong booked I.N.");

    GPIO::clearReservations();
    InputNumber::clearBook();
    try
    {
        PCF8574Expander empty;
        validate::GPIOExpander<PCF8574Pin>(empty);
        assert(false && "Empty GPIO expander accepted");
    }
    catch (empty_input_number_set)
    {
    }
}

void RotaryEncoderTest()
{
    std::cout << "- RotaryEncoder -" << std::endl;
    GPIO::clearReservations();
    InputNumber::clearBook();

    validate::rotaryEncoder(0, 1, 2, 3);
    checkReservation(0, "Failed reservation GPIO 0");
    checkReservation(1, "Failed reservation GPIO 1");
    assert((InputNumber::booked() == 0b1100) && "Wrong booked I.N.");

    GPIO::clearReservations();
    InputNumber::clearBook();
    try
    {
        InputNumber none;
        validate::rotaryEncoder(3, 4, none, none);
        assert(false && "Empty rotary encoder accepted");
    }
    catch (empty_input_number_set)
    {
    }

    GPIO::clearReservations();
    InputNumber::clearBook();
    try
    {
        InputNumber none;
        validate::rotaryEncoder(3, 4, none, 10);
        assert(false && "Unspecified CW input number accepted");
    }
    catch (std::runtime_error)
    {
    }

    GPIO::clearReservations();
    InputNumber::clearBook();
    try
    {
        InputNumber none;
        validate::rotaryEncoder(3, 4, 10, none);
        assert(false && "Unspecified CCW input number accepted");
    }
    catch (std::runtime_error)
    {
    }

    GPIO::clearReservations();
    InputNumber::clearBook();
    try
    {
        InputNumber none;
        validate::rotaryEncoder(3, 4, 10, 10);
        assert(false && "CW==CCW input numbers accepted");
    }
    catch (std::runtime_error)
    {
    }
}

void ButtonTest()
{
    std::cout << "- Single button -" << std::endl;
    GPIO::clearReservations();
    InputNumber::clearBook();

    validate::button(0, 3);
    checkReservation(0, "Failed reservation GPIO 0");
    assert((InputNumber::booked() == 0b1000) && "Wrong booked I.N.");

    GPIO::clearReservations();
    InputNumber::clearBook();
    try
    {
        InputNumber none;
        validate::button(0, none);
        assert(false && "Unspecified input number accepted");
    }
    catch (std::runtime_error)
    {
    }
}

void RotaryCodedSwitchTest()
{
    std::cout << "- Coded rotary switch -" << std::endl;
    GPIO::clearReservations();
    InputNumber::clearBook();

    RotaryCodedSwitch sw1;
    sw1[0] = 0;
    sw1[7] = 1;
    validate::codedRotarySwitch(sw1, {1, 2, 3});

    GPIO::clearReservations();
    InputNumber::clearBook();
    sw1[8] = 2;
    try
    {
        validate::codedRotarySwitch(sw1, {1, 2, 3});
        assert(false && "Invalid switch position accepted");
    }
    catch (std::runtime_error)
    {
    }

    GPIO::clearReservations();
    InputNumber::clearBook();
    RotaryCodedSwitch sw2;
    sw2[0] = 5;
    sw2[1] = 6;
    sw2[2] = 8;
    sw2[3] = 9;
    validate::codedRotarySwitch(sw2, {1, 2});
    assert((InputNumber::booked() == 0b1101100000) && "Wrong booked I.N.");

    GPIO::clearReservations();
    InputNumber::clearBook();
    RotaryCodedSwitch sw3;
    try
    {
        validate::codedRotarySwitch(sw3, {});
        assert(false && "Empty input pin collection accepted");
    }
    catch (std::runtime_error)
    {
    }

    try
    {
        validate::codedRotarySwitch(sw3, {1, 2, 3, 4, 5, 6, 7, 8, 9});
        assert(false && "Too large input pin collection accepted");
    }
    catch (std::runtime_error)
    {
    }

    validate::codedRotarySwitch(sw3, {1,2,3});
    checkReservation(1,"GPIO pin 1 not reserved");
    checkReservation(2,"GPIO pin 2 not reserved");
    checkReservation(3,"GPIO pin 3 not reserved");
}

int main()
{
    BtnMtxTest();
    MuxTest();
    ShiftRegisterTest();
    GPIOExpanderTest();
    RotaryEncoderTest();
    ButtonTest();
    RotaryCodedSwitchTest();
}