/**
 * @file ui.cpp
 * @author Ángel Fernández Pineda. Madrid. Spain.
 *
 * @date 2022-12-23
 * @brief Support for custom user interfaces
 *
 * @copyright Licensed under the EUPL
 *
 */

//-------------------------------------------------------------------
// Imports
//-------------------------------------------------------------------

#include "SimWheel.hpp"
#include "SimWheelInternals.hpp"
#include "InternalServices.hpp"
#include "freertos/FreeRTOS.h"

#include <vector>
#include <latch>

//-------------------------------------------------------------------
// Event queue
//-------------------------------------------------------------------

#define EVENT_QUEUE_SIZE 24

struct EventQueue
{
    EventQueue(AbstractUserInterface *instance)
    {
        assert(instance != nullptr);
        ui = instance;
    }

    void enqueue(uint8_t eventID, uint8_t data = 0)
    {
        uint8_t queueTailNext = queueTail;
        incQueuePointer(queueTailNext);
        if (queueTailNext == queueHead)
            // Queue is full
            queueTailNext = queueTail;
        eventBuffer[queueTailNext] = eventID;
        dataBuffer[queueTailNext] = data;
        queueTail = queueTailNext;
        // assert(reader != nullptr);
        xTaskNotifyGive(reader);
    }

    bool dequeue(uint8_t &eventID, uint8_t &data)
    {
        bool isNotEmpty = (queueHead != queueTail);
        if (isNotEmpty)
        {
            incQueuePointer(queueHead);
            eventID = eventBuffer[queueHead];
            data = dataBuffer[queueHead];
        }
        return isNotEmpty;
    }

    AbstractUserInterface *ui = nullptr;
    TaskHandle_t reader = nullptr;

private:
    uint8_t eventBuffer[EVENT_QUEUE_SIZE];
    uint8_t dataBuffer[EVENT_QUEUE_SIZE];
    uint8_t queueHead = 0;
    uint8_t queueTail = 0;
    inline void incQueuePointer(uint8_t &pointer)
    {
        pointer = (pointer + 1) % EVENT_QUEUE_SIZE;
    }
};

//-------------------------------------------------------------------
// Globals
//-------------------------------------------------------------------

// Notification daemon
static std::vector<EventQueue *> _ui_queues;
static std::vector<AbstractUserInterface *> _ui_instances;
#define DEFAULT_STACK_SIZE 4 * 1024
static std::latch *_shutdownLatch{nullptr};

// Frame server and telemetry
#define NO_TELEMETRY_TICKS pdMS_TO_TICKS(2000)

// Events queue
#define EVENT_BITE_POINT 1
#define EVENT_CONNECTED 2
#define EVENT_BLE_DISCOVERING 3
#define EVENT_LOW_BATTERY 4
#define EVENT_SAVE_SETTINGS 5
#define EVENT_ROUTED_INPUT 6
#define EVENT_SHUTDOWN 255

//-------------------------------------------------------------------
// Notification daemon
//-------------------------------------------------------------------

void notificationDaemonLoop(void *param)
{
    EventQueue *eventQueue = (EventQueue *)param;
    assert(eventQueue != nullptr);

    // The following does not work:
    // eventQueue->reader = xTaskGetCurrentTaskHandle();
    // So, the reader is set in getReady() after task creation.
    // The following delay is a workaround but not optimal.
    vTaskDelay(pdMS_TO_TICKS(200));

    uint8_t eventID;
    uint8_t eventData;
    uint32_t lastFrameID = 0;
    bool telemetryReceived = false;
    TickType_t previousTelemetryTimestamp = 0;
    TickType_t currentTelemetryTimestamp = 0;

    TickType_t frameServerPeriod = portMAX_DELAY;
    if (eventQueue->ui->getMaxFPS() > 0)
        frameServerPeriod = pdMS_TO_TICKS(1000 / eventQueue->ui->getMaxFPS());
    bool telemetryRequired = eventQueue->ui->requiresECUTelemetry ||
                             eventQueue->ui->requiresPowertrainTelemetry ||
                             eventQueue->ui->requiresGaugeTelemetry ||
                             eventQueue->ui->requiresRaceControlTelemetry;

    eventQueue->ui->onStart();

    TickType_t frameTimestamp = xTaskGetTickCount();
    while (true)
    {
        if (ulTaskNotifyTake(pdTRUE, frameServerPeriod))
        {
            // One or more events should be available

            while (eventQueue->dequeue(eventID, eventData))
                switch (eventID)
                {
                case EVENT_BITE_POINT:
                    eventQueue->ui->onBitePoint(eventData);
                    break;
                case EVENT_BLE_DISCOVERING:
                    eventQueue->ui->onBLEdiscovering();
                    break;
                case EVENT_CONNECTED:
                    eventQueue->ui->onConnected();
                    break;
                case EVENT_LOW_BATTERY:
                    eventQueue->ui->onLowBattery();
                    break;
                case EVENT_SAVE_SETTINGS:
                    eventQueue->ui->onSaveSettings();
                    break;
                case EVENT_ROUTED_INPUT:
                    eventQueue->ui->onUserInput(eventData);
                    break;
                case EVENT_SHUTDOWN:
                    eventQueue->ui->shutdown();
                    _shutdownLatch->count_down();
                    // Shutdown is not reversible
                    vTaskDelay(portMAX_DELAY);
                    break;
                default:
                    break;
                }
        }
        if (frameServerPeriod != portMAX_DELAY)
        {
            if (telemetryRequired)
            {
                currentTelemetryTimestamp = xTaskGetTickCount();
                if (lastFrameID != telemetry::data.frameID)
                {
                    telemetryReceived = true;
                    lastFrameID = telemetry::data.frameID;
                    previousTelemetryTimestamp = currentTelemetryTimestamp;
                    eventQueue->ui->onTelemetryData(
                        (const TelemetryData *)&telemetry::data);
                }
                else if (telemetryReceived &&
                         ((currentTelemetryTimestamp - previousTelemetryTimestamp) >= NO_TELEMETRY_TICKS))
                {
                    telemetryReceived = false;
                    eventQueue->ui->onTelemetryData(nullptr);
                }
            }
            uint32_t elapsedMs =
                (xTaskGetTickCount() - frameTimestamp) / portTICK_RATE_MS;
            eventQueue->ui->serveSingleFrame(elapsedMs);
        }
        frameTimestamp = xTaskGetTickCount();
    }
}

//-------------------------------------------------------------------
// Notifications
//-------------------------------------------------------------------

void notify_bitePoint(uint8_t bp)
{
    for (auto q : _ui_queues)
        q->enqueue(EVENT_BITE_POINT, bp);
}

void notify_connected()
{
    for (auto q : _ui_queues)
        q->enqueue(EVENT_CONNECTED);
}

void notify_BLEdiscovering()
{
    for (auto q : _ui_queues)
        q->enqueue(EVENT_BLE_DISCOVERING);
}

void notify_lowBattery()
{
    for (auto q : _ui_queues)
        q->enqueue(EVENT_LOW_BATTERY);
}

void notify_saved()
{
    for (auto q : _ui_queues)
        q->enqueue(EVENT_SAVE_SETTINGS);
}

void notify_shutdown()
{
    if (_ui_queues.size() > 0)
    {
        for (auto q : _ui_queues)
            q->enqueue(EVENT_SHUTDOWN);
        // Wait for the shutdown command to finish execution in
        // all UI threads
        _shutdownLatch->wait();
    }
}

void internals::ui::routeInput(uint8_t inputNumber)
{
    for (auto q : _ui_queues)
        q->enqueue(EVENT_ROUTED_INPUT, inputNumber);
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Public
//-------------------------------------------------------------------
//-------------------------------------------------------------------

void ui::add(AbstractUserInterface *instance)
{
    if (instance == nullptr)
        throw std::runtime_error(
            "User interface instance is null");
    if (FirmwareService::call().isRunning())
        throw std::runtime_error(
            "Unable to add a user interface instance while running");
    for (auto another_instance : _ui_instances)
        if (another_instance == instance)
            // Already added
            return;
    _ui_instances.push_back(instance);
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Service class
//-------------------------------------------------------------------
//-------------------------------------------------------------------

class UIServiceProvider : public UIService
{
public:
    UIServiceProvider(uint8_t fps)
    {
        _max_fps = fps;
    }

    virtual uint8_t getMaxFPS()
    {
        return _max_fps;
    }

private:
    uint8_t _max_fps = 0;
};

//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Internal
//-------------------------------------------------------------------
//-------------------------------------------------------------------

void internals::ui::getReady()
{
    if ((_ui_instances.size() == 0) || FirmwareService::call().isRunning())
        // nothing to do here
        return;

    // Compute telemetry requirements
    bool requiresPowertrainTelemetry = false;
    bool requiresECUTelemetry = false;
    bool requiresRaceControlTelemetry = false;
    bool requiresGaugeTelemetry = false;
    uint8_t max_fps = 0;
    for (auto instance : _ui_instances)
    {
        requiresPowertrainTelemetry =
            requiresPowertrainTelemetry ||
            instance->requiresPowertrainTelemetry;
        requiresECUTelemetry =
            requiresECUTelemetry ||
            instance->requiresECUTelemetry;
        requiresRaceControlTelemetry =
            requiresRaceControlTelemetry ||
            instance->requiresRaceControlTelemetry;
        requiresGaugeTelemetry |=
            requiresGaugeTelemetry ||
            instance->requiresGaugeTelemetry;
        bool thisInstanceUsesTelemetry =
            instance->requiresPowertrainTelemetry ||
            instance->requiresECUTelemetry ||
            instance->requiresRaceControlTelemetry ||
            instance->requiresGaugeTelemetry;
        if (thisInstanceUsesTelemetry && (instance->getMaxFPS() > max_fps))
            max_fps = instance->getMaxFPS();
    }

    // Set new device capabilities
    DeviceCapabilities::setFlag(
        DeviceCapability::TELEMETRY_POWERTRAIN,
        requiresPowertrainTelemetry);
    DeviceCapabilities::setFlag(
        DeviceCapability::TELEMETRY_ECU,
        requiresECUTelemetry);
    DeviceCapabilities::setFlag(
        DeviceCapability::TELEMETRY_RACE_CONTROL,
        requiresRaceControlTelemetry);
    DeviceCapabilities::setFlag(
        DeviceCapability::TELEMETRY_GAUGES,
        requiresGaugeTelemetry);

    // Subscribe to internal events
    OnBitePoint.subscribe(notify_bitePoint);
    OnConnected.subscribe(notify_connected);
    OnDisconnected.subscribe(notify_BLEdiscovering);
    OnLowBattery.subscribe(notify_lowBattery);
    OnSettingsSaved.subscribe(notify_saved);
    OnShutdown.subscribe(notify_shutdown);

    // Inject the service class
    UIService::inject(new UIServiceProvider(max_fps));

    // Create a latch for shutdown
    _shutdownLatch = new std::latch(_ui_instances.size());

    // Create all frameserver daemons
    TaskHandle_t task;
    for (auto ui : _ui_instances)
    {
        EventQueue *q = new EventQueue(ui);
        _ui_queues.push_back(q);
        uint16_t stack_size = ui->getStackSize();
        if (stack_size == 0)
            stack_size = DEFAULT_STACK_SIZE;
        xTaskCreate(
            notificationDaemonLoop,
            "UI",
            stack_size,
            (void *)q,
            tskIDLE_PRIORITY,
            &task);
        if (task == nullptr)
            throw std::runtime_error("Unable to create UI daemon");
        q->reader = task;
    }
    _ui_instances.clear();
    _ui_instances.shrink_to_fit();
}