/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-12-24
 * @brief Implement notifications through the USB serial interface (for testing)
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SerialNotification.h"
#include "SimWheel.h"
#include <HardwareSerial.h>

void SerialNotificationImpl::onStart()
{
    Serial.println("(DEVICE READY)");
}

void SerialNotificationImpl::onBitePoint()
{
    Serial.print("(BITE POINT: ");
    Serial.print((uint8_t)userSettings::bitePoint);
    Serial.println(")");
}

void SerialNotificationImpl::onConnected()
{
    Serial.println("(BLE CONNECTED)");
}

void SerialNotificationImpl::onBLEdiscovering()
{
    Serial.println("(BLE DISCOVERING)");
}

void SerialNotificationImpl::onLowBattery()
{
    Serial.println("(LOW BATTERY)");
}

void SerialNotificationImpl::serveSingleFrame()
{
    Serial.println("(FRAME)");
}

void SerialNotificationImpl::selectNextPage()
{
    Serial.println("(Next page)");
}

void SerialNotificationImpl::selectPreviousPage()
{
    Serial.println("(Previous page)");
}