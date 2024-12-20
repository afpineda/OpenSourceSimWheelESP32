/**
 * @file SerialNotification.h
 *
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-12-24
 * @brief Implement notifications through the USB serial interface (for testing)
 *
 * @copyright Licensed under the EUPL
 *
 */

#ifndef __SERIAL_NOTIFICATION_H__
#define __SERIAL_NOTIFICATION_H__

#include "SimWheelTypes.h"

//-------------------------------------------------------------------

/**
 * @brief User notifications in the serial interface
 *        (for testing)
 *
 */
class SerialNotificationImpl : public AbstractUserInterface
{
public:
    virtual void onStart() override;
    virtual void onBitePoint() override;
    virtual void onConnected() override;
    virtual void onTelemetryData(const telemetryData_t *data) override;
    virtual void onBLEdiscovering() override;
    virtual void onLowBattery() override;
    virtual void serveSingleFrame(uint32_t elapsedMs) override;
};

//-------------------------------------------------------------------

#define SERIAL_DISPLAY_BUFFER_SIZE 64

/**
 * @brief Telemetry display in the serial interface
 *        (for testing)
 *
 */
class SerialTelemetryDisplay : public AbstractUserInterface
{
private:
    char displayBuffer[SERIAL_DISPLAY_BUFFER_SIZE];
    bool displayOff = true;
    uint32_t lastFrameID = 0;

public:
    SerialTelemetryDisplay();
    virtual void onTelemetryData(const telemetryData_t *data) override;
    virtual void serveSingleFrame(uint32_t elapsedMs) override;
};

#endif
