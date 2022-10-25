/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Implementation of the `uiManager` namespace
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "SimWheel.h"
// #include <FreeRTOS.h>

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

#define OLED_BUFFER_SIZE (128*128)/8 // Maximun buffer size: 128x128 pixels

typedef struct
{
    SemaphoreHandle_t mutex;
    //bool updated;
    bool visible;
    uint8_t buffer[OLED_BUFFER_SIZE];
    esp_timer_handle_t autoHideTimer;
} screenHandler_t;

static SemaphoreHandle_t displayMutex = nullptr;
static screenHandler_t scrHandler[SCR_PRIORITY_COUNT];
static int lastVisiblePriority = SCR_PRIORITY_COUNT;

// ----------------------------------------------------------------------------
// Autoclear display feature
// ----------------------------------------------------------------------------

void autoHideCallback(void *param)
{
    uiManager::hide((screenPriority_t)(int)param);
}

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void uiManager::begin()
{
    if (displayMutex == nullptr)
    {
        displayMutex = xSemaphoreCreateMutex();
        if (displayMutex == nullptr)
            abort();

        esp_timer_create_args_t timerArgs;
        timerArgs.callback = &autoHideCallback;
        timerArgs.arg = nullptr;
        timerArgs.name = nullptr;
        timerArgs.dispatch_method = ESP_TIMER_TASK;

        for (int i = 0; i < SCR_PRIORITY_COUNT; i++)
        {
            // scrHandler[i].updated = false;
            scrHandler[i].visible = false;
            scrHandler[i].mutex = xSemaphoreCreateMutex();
            if (scrHandler[i].mutex == nullptr)
                abort();
            timerArgs.arg = (void *)i;
            ESP_ERROR_CHECK(esp_timer_create(&timerArgs, &(scrHandler[i].autoHideTimer)));
        }
    }
}

// ----------------------------------------------------------------------------
// UI Sync
// ----------------------------------------------------------------------------

void doDisplay(int requestedPriority)
{
    // Serial.print("DODISPLAY REQUESTED: ");
    // Serial.print(requestedPriority);
    // Serial.print("  DODISPLAY IN LASTVISIBLE: ");
    // Serial.println(lastVisiblePriority);
    xSemaphoreTake(displayMutex, portMAX_DELAY);
    if (requestedPriority <= lastVisiblePriority)
    {
        lastVisiblePriority = requestedPriority;
        ui::display(scrHandler[requestedPriority].buffer);
    }
    // Serial.print("  DODISPLAY OUT LASTVISIBLE: ");
    // Serial.println(lastVisiblePriority);
    xSemaphoreGive(displayMutex);
}

void doHide()
{
    xSemaphoreTake(displayMutex, portMAX_DELAY);
    int nextVisiblePriority = 0;
    while ((nextVisiblePriority < SCR_PRIORITY_COUNT) && !scrHandler[nextVisiblePriority].visible)
        nextVisiblePriority++;
    if (nextVisiblePriority >= SCR_PRIORITY_COUNT) 
    {
        if (lastVisiblePriority<SCR_PRIORITY_COUNT)
            ui::clear();
    }
    else if (nextVisiblePriority!=lastVisiblePriority)
    {
        ui::display(scrHandler[nextVisiblePriority].buffer);
    }
    lastVisiblePriority = nextVisiblePriority; 
    xSemaphoreGive(displayMutex);
}

uint8_t *uiManager::enterDisplay(screenPriority_t priority)
{
    xSemaphoreTake(scrHandler[priority].mutex, portMAX_DELAY);
    esp_timer_stop(scrHandler[priority].autoHideTimer);
    return scrHandler[priority].buffer;
}

void uiManager::exitDisplay(screenPriority_t priority, bool autoHide)
{
    scrHandler[priority].visible = true;
    doDisplay(priority);
    if (autoHide)
    {
        esp_timer_start_once(scrHandler[priority].autoHideTimer, DEFAULT_UI_TIME_us);
    }
    xSemaphoreGive(scrHandler[priority].mutex);
}

void uiManager::hide(screenPriority_t priority)
{
    xSemaphoreTake(scrHandler[priority].mutex, portMAX_DELAY);
    esp_timer_stop(scrHandler[priority].autoHideTimer);
    scrHandler[priority].visible = false;
    doHide();
    xSemaphoreGive(scrHandler[priority].mutex);
}

uint8_t *uiManager::getFrameServerBuffer()
{
    scrHandler[SCR_FRAMESERVER_PRIORITY].visible = true;
    return scrHandler[SCR_FRAMESERVER_PRIORITY].buffer;
}

void uiManager::unsafeDisplayFrameServerBuffer()
{
    doDisplay(SCR_FRAMESERVER_PRIORITY);
}