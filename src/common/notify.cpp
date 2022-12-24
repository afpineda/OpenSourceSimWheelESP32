
/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-12-23
 * @brief Implementation of the `notify` namespace
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include "SimWheel.h"
#include "SimWheelTypes.h"

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

static AbstractNotificationInterface *impl;

// ----------------------------------------------------------------------------
// Loop
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void notify::begin(AbstractNotificationInterface *implementation)
{
   // if (implementation && (notificationTask!=nullptr))
    if (!impl)
    {
        impl = implementation;
        // queue = xQueueCreate(64, 1);
        // xTaskCreate(
        //     notificationLoop, 
        //     "notify", 
        //     NOTIFICATION_TASK_STACK_SIZE, 
        //     (void *)implementation, 
        //     UI_TASK_PRIORITY, 
        //     &notificationTask);
    }
}

// ----------------------------------------------------------------------------
// Notifications
// ----------------------------------------------------------------------------

void notify::bitePoint(clutchValue_t aBitePoint)
{
    if (impl)
        impl->bitePoint(aBitePoint);
}

void notify::connected()
{
    if (impl)
        impl->connected();
}

void notify::BLEdiscovering(){
    if (impl)
        impl->BLEdiscovering();
}

void notify::powerOn() {
    if (impl)
        impl->powerOn();
}

void notify::powerOff() {
    if (impl)
        impl->powerOff();
}

void notify::lowBattery() {
    if (impl)
        impl->powerOff();
}