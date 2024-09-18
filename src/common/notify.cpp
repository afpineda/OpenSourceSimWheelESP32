
/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-12-23
 * @brief Implementation of the `notify` namespace
 *
 * @copyright Licensed under the EUPL
 *
 */

#include "SimWheel.h"
#include "SimWheelTypes.h"

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

// Notification daemon
#define DEFAULT_STACK_SIZE 1*1024 + 512
static TaskHandle_t notificationDaemon = nullptr;
static notificationImplementorsArray_t implementorArray;

// Frame server
static TickType_t frameServerPeriod = portMAX_DELAY;

// Events queue
#define EVENT_QUEUE_SIZE 64
#define EVENT_BITE_POINT 1
#define EVENT_CONNECTED 2
#define EVENT_BLE_DISCOVERING 3
#define EVENT_LOW_BATTERY 4
static uint8_t eventBuffer[EVENT_QUEUE_SIZE];
static uint8_t queueHead = 0;
static uint8_t queueTail = 0;

// ----------------------------------------------------------------------------
// Events queue
// ----------------------------------------------------------------------------

inline void incQueuePointer(uint8_t &pointer)
{
    pointer = (pointer + 1) % EVENT_QUEUE_SIZE;
}

void eventPush(uint8_t eventID)
{
    uint8_t queueTailNext = queueTail;
    incQueuePointer(queueTailNext);
    if (queueTailNext == queueHead)
        // Queue is full, overwrite last event
        queueTailNext = queueTail;
    eventBuffer[queueTailNext] = eventID;
    queueTail = queueTailNext;
    xTaskNotifyGive(notificationDaemon);
}

bool eventPop(uint8_t &eventID)
{
    bool isNotEmpty = (queueHead != queueTail);
    if (isNotEmpty)
    {
        eventID = eventBuffer[queueHead];
        incQueuePointer(queueHead);
    }
    return isNotEmpty;
}

// ----------------------------------------------------------------------------
// Notification daemon
// ----------------------------------------------------------------------------

void notificationDaemonLoop(void *param)
{
    uint8_t eventID;

    for (AbstractNotificationInterface *impl : implementorArray)
        impl->onStart();

    while (true)
    {
        if (ulTaskNotifyTake(pdTRUE, frameServerPeriod))
        {
            // One or more events should be available
            while (eventPop(eventID))
                for (AbstractNotificationInterface *impl : implementorArray)
                    switch (eventID)
                    {
                    case EVENT_BITE_POINT:
                        impl->onBitePoint();
                        break;
                    case EVENT_BLE_DISCOVERING:
                        impl->onBLEdiscovering();
                        break;
                    case EVENT_CONNECTED:
                        impl->onConnected();
                        break;
                    case EVENT_LOW_BATTERY:
                        impl->onLowBattery();
                    }
        }
        if (frameServerPeriod != portMAX_DELAY)
            for (AbstractNotificationInterface *impl : implementorArray)
                impl->serveSingleFrame();
    }
}

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void notify::begin(
    notificationImplementorsArray_t implementors,
    uint8_t framesPerSecond,
    uint16_t stackSize)
{
    // Check parameters
    if (implementors.size() == 0)
    {
        log_e("notify::begin() called with an empty set of implementors");
        abort();
    }
    for (int i = 0; i < implementors.size(); i++)
        if (implementors[i] == nullptr)
        {
            log_e("notify::begin() called with a null pointer implementor");
            abort();
        }

    // Initialize
    if (notificationDaemon == nullptr)
    {
        if (stackSize == 0)
            stackSize = DEFAULT_STACK_SIZE;
        if (framesPerSecond > 0)
            frameServerPeriod = pdMS_TO_TICKS(1000 / framesPerSecond);
        else
            frameServerPeriod = portMAX_DELAY;
        implementorArray = implementors;
        xTaskCreate(
            notificationDaemonLoop,
            "notify",
            stackSize,
            nullptr,
            tskIDLE_PRIORITY,
            &notificationDaemon);
        if (notificationDaemon == nullptr)
        {
            log_e("Unable to create notifications daemon");
            abort();
        }
    }
    else
        log_w("notify::begin() called twice");
}

// ----------------------------------------------------------------------------
// Notifications
// ----------------------------------------------------------------------------

void notify::bitePoint()
{
    if (notificationDaemon)
        eventPush(EVENT_BITE_POINT);
}

void notify::connected()
{
    if (notificationDaemon)
        eventPush(EVENT_CONNECTED);
}

void notify::BLEdiscovering()
{
    if (notificationDaemon)
        eventPush(EVENT_BLE_DISCOVERING);
}

void notify::lowBattery()
{
    if (notificationDaemon)
        eventPush(EVENT_LOW_BATTERY);
}