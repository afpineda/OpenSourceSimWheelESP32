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

#ifndef __SERIALNOTIFICATION_H__
#define __SERIALNOTIFICATION_H__

#include <Arduino.h>
#include "SimWheelTypes.h"

class SerialNotificationImpl : public AbstractNotificationInterface
{
public:
    /**
     * @brief Called once at initialization. Called from the main thread
     *
     */
    virtual void begin() override;

    /**
     * @brief Notify a change in current bite point. Called in a separate thread.
     *
     * @param bitePoint A bite point value
     */
    virtual void bitePoint(clutchValue_t bitePoint) override;

    /**
     * @brief Notify device is connected. Called in a separate thread.
     *
     */
    virtual void connected() override;

    /**
     * @brief Notify device is in discovery mode. Called in a separate thread.
     *
     */
    virtual void BLEdiscovering() override;

    /**
     * @brief Notify device is about to power off/deep sleep.
     *        Called in a separate thread.
     *
     */
    virtual void powerOff() override;

    /**
     * @brief Notify low battery. Called in a separate thread.
     *
     */
    virtual void lowBattery() override;
};

#endif
