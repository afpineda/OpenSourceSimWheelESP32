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

void SerialNotificationImpl::serveSingleFrame(uint32_t elapsedMs)
{
    Serial.printf("(FRAME %u MS)\n",elapsedMs);
}

uint8_t SerialNotificationImpl::getPageCount()
{
    return 4;
}

void SerialNotificationImpl::setPageIndex(uint8_t pageIndex)
{
    Serial.printf("(SET PAGE %u)\n", pageIndex);
}

void SerialNotificationImpl::onTelemetryData(const telemetryData_t *data)
{
    if (!data)
        Serial.println("(TELEMETRY OFF)");
    else
    {
        Serial.printf("(TELEMETRY ID %d)\n", data->frameID);
    }
}