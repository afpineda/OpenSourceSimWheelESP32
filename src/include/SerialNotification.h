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

class SerialNotificationImpl : public AbstractNotificationInterface
{
public:
    virtual void onStart() override;
    virtual void onBitePoint() override;
    virtual void onConnected() override;
    virtual void onBLEdiscovering() override;
    virtual void onLowBattery() override;
    virtual void serveSingleFrame() override;
};

#endif
