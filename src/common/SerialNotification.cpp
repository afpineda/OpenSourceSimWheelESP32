/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-12-24
 * @brief Implement notifications through the USB serial interface (for testing)
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include "SerialNotification.h"

void SerialNotificationImpl::begin()
{
    Serial.println("(DEVICE READY)");
}

void SerialNotificationImpl::bitePoint(clutchValue_t bitePoint)
{
    Serial.print("(BITE POINT: ");
    Serial.print((uint8_t)bitePoint);
    Serial.println(")");
}

void SerialNotificationImpl::connected()
{
    Serial.println("(BLE CONNECTED)");
}

void SerialNotificationImpl::BLEdiscovering()
{
    Serial.println("(BLE DISCOVERING)");
}

void SerialNotificationImpl::powerOff()
{
    Serial.println("(POWER OFF)");
}

void SerialNotificationImpl::lowBattery()
{
    Serial.println("(LOW BATTERY)");
}