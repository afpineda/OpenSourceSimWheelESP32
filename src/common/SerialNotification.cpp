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

//-------------------------------------------------------------------
// SerialNotificationImpl
//-------------------------------------------------------------------

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
    Serial.printf("(FRAME %lu MS)\n", elapsedMs);
}

void SerialNotificationImpl::onTelemetryData(const telemetryData_t *data)
{
    if (!data)
        Serial.println("(TELEMETRY OFF)");
    else
    {
        Serial.printf("(TELEMETRY ID %lu)\n", data->frameID);
    }
}

//-------------------------------------------------------------------
// SerialTelemetryDisplay
//-------------------------------------------------------------------

SerialTelemetryDisplay::SerialTelemetryDisplay()
{
    requiresPowertrainTelemetry = true;
    displayBuffer[0] = 0;
}

void SerialTelemetryDisplay::onTelemetryData(const telemetryData_t *data)
{
    if (data)
    {
        displayOff = false;
        lastFrameID = data->frameID;
        snprintf(
            displayBuffer,
            SERIAL_DISPLAY_BUFFER_SIZE,
            "Frame=%lu,RPM=%u,Speed=%u",
            data->frameID,
            data->powertrain.rpm,
            data->powertrain.speed);
    }
    else
        displayOff = true;
}

void SerialTelemetryDisplay::serveSingleFrame(uint32_t elapsedMs)
{
    if (!displayOff)
    {
        Serial.printf("Elapsed: %lu. Telemetry: %s\n", elapsedMs, displayBuffer);
        Serial.flush();
    }
}
