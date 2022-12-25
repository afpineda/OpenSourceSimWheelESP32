
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
#define FRAMESERVER_PERIOD_TICKS pdMS_TO_TICKS(20) // 50FPS

// ----------------------------------------------------------------------------
// Loops
// ----------------------------------------------------------------------------

void notificationLoop(void *param)
{
    AbstractNotificationInterface *impl = (AbstractNotificationInterface *)param;
    AbstractNotificationInterface *chain;
    notifyEvent_t event;

    while (true)
    {
        if (xQueueReceive(queue, &event, portMAX_DELAY))
        {
            chain = impl;
            while (chain)
            {

                switch (event.id)
                {
                case ID_BITEPOINT:
                    chain->bitePoint((clutchValue_t)event.data[0]);
                    break;

                case ID_CONNECTED:
                    chain->connected();
                    break;

                case ID_BLEDISCOVERING:
                    chain->BLEdiscovering();
                    break;

                case ID_POWERON:
                    chain->powerOn();
                    break;

                case ID_POWEROFF:
                    chain->powerOff();
                    break;

                case ID_LOWBATTERY:
                    chain->lowBattery();
                    break;

                default:
                    break;
                } // end switch
                chain = chain->nextInChain;
            } // end while(chain)
        }     // end if
    }         // end while(true)
}

void frameServerLoop(void *param)
{
    AbstractFrameServerInterface *impl = (AbstractFrameServerInterface *)param;
    AbstractFrameServerInterface *chain;
    notifyEvent_t event;

    TickType_t period = pdMS_TO_TICKS (1000 / impl->getTargetFPS());
    if (period<FRAMESERVER_PERIOD_TICKS)
        period = FRAMESERVER_PERIOD_TICKS;
    auto xLastWakeTime = xTaskGetTickCount();

    while (true)
    {
        chain = impl;
        if ((uxQueueMessagesWaiting(queue) > 0) && xQueueReceive(queue, &event, portMAX_DELAY))
        {
            while (chain)
            {

                switch (event.id)
                {
                case ID_BITEPOINT:
                    chain->bitePoint((clutchValue_t)event.data[0]);
                    break;

                case ID_CONNECTED:
                    chain->connected();
                    break;

                case ID_BLEDISCOVERING:
                    chain->BLEdiscovering();
                    break;

                case ID_POWERON:
                    chain->powerOn();
                    break;

                case ID_POWEROFF:
                    chain->powerOff();
                    break;

                case ID_LOWBATTERY:
                    chain->lowBattery();
                    break;

                default:
                    break;
                } // end switch
                chain = chain->nextInChain;
            } // end while(chain)
        }
        else
        {
            while (chain)
            {
                chain->serveSingleFrame();
                chain = chain->nextInChain;
            }
            vTaskDelayUntil(&xLastWakeTime, period);
        } // end if-then-else
    }     // end while(true)
}

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void notify_init(TaskFunction_t loop, void *param)
{
    queue = xQueueCreate(64, sizeof(notifyEvent_t));
    if (queue == nullptr)
    {
        log_e("Unable to create notifications queue");
        abort();
    }
    xTaskCreate(
        loop,
        "notify",
        NOTIFICATION_TASK_STACK_SIZE,
        param,
        tskIDLE_PRIORITY,
        &notificationTask);
    if (notificationTask == nullptr)
    {
        log_e("Unable to create notifications task");
        abort();
    }
}

void notify::begin(AbstractNotificationInterface *implementation)
{
    if (implementation && (notificationTask == nullptr) && (queue == nullptr))
    {
        notify_init(notificationLoop, (void *)implementation);
        implementation->begin();
    }
}

void notify::begin(AbstractFrameServerInterface *implementation)
{
    if (implementation && (notificationTask == nullptr) && (queue == nullptr))
    {
        notify_init(frameServerLoop, (void *)implementation);
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