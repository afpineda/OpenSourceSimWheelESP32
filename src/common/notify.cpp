
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

#define NOTIFICATION_TASK_STACK_SIZE 1 * 1024
static TaskHandle_t notificationTask = nullptr;
static QueueHandle_t queue = nullptr;

typedef struct
{
    uint8_t id;
    uint8_t data[1];
} notifyEvent_t;

#define ID_BITEPOINT 1
#define ID_CONNECTED 2
#define ID_BLEDISCOVERING 3
#define ID_POWERON 4
#define ID_POWEROFF 5
#define ID_LOWBATTERY 6

#define MAX_WAIT_TICKS DEBOUNCE_TICKS * 2

// ----------------------------------------------------------------------------
// Loop
// ----------------------------------------------------------------------------

void notificationLoop(void *param)
{
    AbstractNotificationInterface *impl = (AbstractNotificationInterface *)param;
    notifyEvent_t event;
    while (true)
    {
        if (xQueueReceive(queue, &event, portMAX_DELAY))
        {
            switch (event.id)
            {
            case ID_BITEPOINT:
                impl->bitePoint((clutchValue_t)event.data[0]);
                break;

            case ID_CONNECTED:
                impl->connected();
                break;

            case ID_BLEDISCOVERING:
                impl->BLEdiscovering();
                break;

            case ID_POWERON:
                impl->powerOn();
                break;

            case ID_POWEROFF:
                impl->powerOff();
                break;

            case ID_LOWBATTERY:
                impl->lowBattery();
                break;

            default:
                break;
            } // end switch
        }     // end if
    }         // end while
}

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void notify::begin(AbstractNotificationInterface *implementation)
{
    if (implementation && (notificationTask == nullptr) && (queue == nullptr))
    {
        queue = xQueueCreate(64, sizeof(notifyEvent_t));
        if (queue == nullptr)
        {
            log_e("Unable to create notifications queue");
            abort();
        }
        xTaskCreate(
            notificationLoop,
            "notify",
            NOTIFICATION_TASK_STACK_SIZE,
            (void *)implementation,
            UI_TASK_PRIORITY,
            &notificationTask);
        if (notificationTask == nullptr)
        {
            log_e("Unable to create notifications task");
            abort();
        }
        implementation->begin();
    }
}

// ----------------------------------------------------------------------------
// Notifications
// ----------------------------------------------------------------------------

void notify::bitePoint(clutchValue_t aBitePoint)
{
    if (queue)
    {
        notifyEvent_t event;
        event.id = ID_BITEPOINT;
        event.data[0] = (uint8_t)aBitePoint;
        xQueueSend(queue, &event, MAX_WAIT_TICKS);
    }
}

void notify::connected()
{
    if (queue)
    {
        notifyEvent_t event;
        event.id = ID_CONNECTED;
        xQueueSend(queue, &event, MAX_WAIT_TICKS);
    }
}

void notify::BLEdiscovering()
{
    if (queue)
    {
        notifyEvent_t event;
        event.id = ID_BLEDISCOVERING;
        xQueueSend(queue, &event, MAX_WAIT_TICKS);
    }
}

void notify::powerOn()
{
    if (queue)
    {
        notifyEvent_t event;
        event.id = ID_POWERON;
        xQueueSend(queue, &event, MAX_WAIT_TICKS);
    }
}

void notify::powerOff()
{
    if (queue)
    {
        notifyEvent_t event;
        event.id = ID_POWEROFF;
        xQueueSend(queue, &event, MAX_WAIT_TICKS);
    }
}

void notify::lowBattery()
{
    if (queue)
    {
        notifyEvent_t event;
        event.id = ID_LOWBATTERY;
        xQueueSend(queue, &event, MAX_WAIT_TICKS);
    }
}