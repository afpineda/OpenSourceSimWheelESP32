/**
 * @file GenericCodedSwitchTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-06-30
 * @brief Integration test. See [Readme](./README.md)
 *
 * @copyright Licensed under the EUPL
 *
 */

// #include "Testing.hpp"
#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"

#include <HardwareSerial.h>

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

FakeInput fakeInput;
uint64_t _inputsLow;
#define WAIT_TICKS pdMS_TO_TICKS(120)
#define SW1 10
#define SW2 11
#define SW4 12

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

bool internals::hid::isConnected() { return true; }
bool internals::hid::supportsCustomHardwareID() { return false; }
void internals::hid::reportChangeInConfig() {}
void internals::hid::reportBatteryLevel(int batteryLevel) {}

void internals::hid::reset()
{
    _inputsLow = 0ULL;
}

void internals::hid::begin(
    std::string deviceName,
    std::string deviceManufacturer,
    bool enableAutoPowerOff,
    uint16_t vendorID,
    uint16_t productID) {}

void internals::hid::reportInput(
    uint64_t inputsLow,
    uint64_t inputsHigh,
    uint8_t POVstate,
    uint8_t leftAxis,
    uint8_t rightAxis,
    uint8_t clutchAxis)
{
    _inputsLow = inputsLow;
}

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

void checkInputs(uint8_t inputNumber)
{
    uint64_t inputsLow = (1ULL << inputNumber);
    vTaskDelay(WAIT_TICKS);
    if (inputsLow != _inputsLow)
    {
        Serial.println("MISMATCH in inputsLow");
        Serial.printf("Expected: %llu, found: %llu\n", inputsLow, _inputsLow);
    }
}

void send(bool sw1, bool sw2, bool sw4)
{
    fakeInput.clear();
    if (sw1)
        fakeInput.press(SW1);
    if (sw2)
        fakeInput.press(SW2);
    if (sw4)
        fakeInput.press(SW4);
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    try
    {
        InputNumber in1 = SW1;
        InputNumber in2 = SW2;
        InputNumber in4 = SW4;
        in1.book();
        in2.book();
        in4.book();
        CodedSwitch8 codedSw;
        codedSw[0] = 10;
        codedSw[1] = 20;
        codedSw[2] = 21;
        codedSw[3] = 27;
        codedSw[4] = 32;
        codedSw[5] = 44;
        codedSw[6] = 51;
        codedSw[7] = 62;
        inputHub::codedSwitch::add(SW1, SW2, SW4, codedSw);

        internals::inputs::addFakeInput(&fakeInput);
        inputHub::clutch::inputs(8, 9); // Required as fakeInput has analog axes
        internals::inputs::getReady();
        internals::inputHub::getReady();
        internals::hid::common::getReady();
        internals::inputMap::getReady();
        OnStart::notify();

        Serial.println("-- GO --");
        for (uint8_t position = 0; position < 8; position++)
        {
            bool sw1 = (position & 0b001);
            bool sw2 = (position & 0b010);
            bool sw4 = (position & 0b100);
            send(sw1, sw2, sw4);
            checkInputs(codedSw[position]);
        }
    }
    catch (std::exception &e)
    {
        Serial.println("EXCEPTION:");
        Serial.println(e.what());
        for (;;)
            ;
    }
    Serial.println("-- DONE --");
    Serial.println("(reset required)");
}

void loop()
{
}