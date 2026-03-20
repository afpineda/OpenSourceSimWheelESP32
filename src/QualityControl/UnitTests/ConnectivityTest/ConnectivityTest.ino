/**
 * @file ConnectivityTest.ino
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-09-21
 *
 * @brief Test connectivity options
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include "HID_definitions.hpp"

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

void onConnectedCallback()
{
    Serial.printf("*** CONNECTED ***\n");
}

void onDisconnectedCallback()
{
    Serial.printf("*** DISCONNECTED ***\n");
}

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

Connectivity choose()
{
    int chr = '?';
    do
    {
        if (chr == '?')
        {
            Serial.println("0 = Connectivity::BLE");
            Serial.println("1 = Connectivity::USB");
            Serial.println("2 = Connectivity::USB_BLE");
            Serial.println("3 = Connectivity::USB_BLE_EXCLUSIVE");
            Serial.println("");
        }
        else if (chr == '0')
            return Connectivity::BLE;
        else if (chr == '1')
            return Connectivity::USB;
        else if (chr == '2')
            return Connectivity::USB_BLE;
        else if (chr == '3')
            return Connectivity::USB_BLE_EXCLUSIVE;

        chr = Serial.read();
    } while (true);
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    Serial.printf("--START--\n");
    InputNumber::bookAll();
    OnConnected.subscribe(onConnectedCallback);
    OnDisconnected.subscribe(onDisconnectedCallback);

    hid::connectivity(choose());
    Serial.printf("--GO--\n");

    hid::configure(
        "ConnTest",
        "Mamandurrio",
        false,
        TEST_HARDWARE_ID,
        TEST_HARDWARE_ID);
    internals::hid::common::getReady();
    OnStart();
}

//------------------------------------------------------------------

void loop()
{
}
