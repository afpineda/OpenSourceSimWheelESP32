/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Implmentation of the `ui` namespace through the
 *        [ss_oled](https://www.arduino.cc/reference/en/libraries/ss_oled/)
 *        library
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "SimWheel.h"
#include <ss_oled.h>
#include "strings.h"
#include <Preferences.h>

using namespace uiManager;

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

// Related to the OLED
static SSOLED ssoled;
static bool available = false;
static int boldFontBestFit;

static const int fontWidth[] = {6, 8, 12, 16, 16}; // in pixels
static const int fontHeight[] = {8, 8, 16, 32};

int SCREEN_WIDTH = 128;
int SCREEN_HEIGHT = 64;

#define MODAL_BLINK_RATE_TICKS 100 / portTICK_RATE_MS
#define BATTERY_LOW_ICON_WIDTH 32
#define BATTERY_LOW_ICON_HEIGHT 28

// Related to the frame server
#define FRAMESERVER_STACK_SIZE 4096
#define targetFrameTime (33 / portTICK_RATE_MS) // 30 FPS
static bool frameServerEnabled = false;
static TaskHandle_t frameServerDaemon = nullptr;
#define PREFS_NAMESPACE "ui"
#define KEY_FRAMESERVER "fs"
void frameServerInternalSetEnabled(bool state); // forward decl

// ----------------------------------------------------------------------------
// Internal macro
// ----------------------------------------------------------------------------

inline int centerTextHorizontal(int fontSize, int charCount)
{
    return (SCREEN_WIDTH - (fontWidth[fontSize] * charCount)) / 2;
}

inline int centerTextVertical(int fontSize, int lineCount = 1)
{
    int result = (SCREEN_HEIGHT - (fontHeight[fontSize] * lineCount)) / 16;
    if (result == 0)
        result = 1;
    return result;
}

inline int rightTextAlign(int fontSize, int charCount)
{
    return (SCREEN_WIDTH - (fontWidth[fontSize] * charCount));
}

inline int bottomTextAlign(int fontSize, int lineCount = 1)
{
    return (SCREEN_HEIGHT - (fontHeight[fontSize] * lineCount)) / 8;
}

inline void drawTitle(const char *text, int textColor = 0)
{
    oledRectangle(&ssoled, 0, 0, SCREEN_WIDTH - 1, 8, !textColor, 1);
    oledWriteString(&ssoled, 0, 0, 0, (char *)text, FONT_8x8, !textColor, 0);
}

inline void drawScaleBar(int screenYPos, int value, int min, int max)
{
    int x = map(value, min, max, 3, SCREEN_WIDTH - 4);
    oledRectangle(&ssoled, 0, screenYPos, SCREEN_WIDTH - 1, screenYPos + 7, 1, 1);
    oledRectangle(&ssoled, 3, screenYPos + 2, x, screenYPos + 5, 0, 1);
}

inline void drawTextBestFit(const char *text, int row = -1, int color = 1)
{
    if (text != nullptr)
    {
        int textlen = strlen(text);
        int font = ((textlen * fontWidth[boldFontBestFit]) > SCREEN_WIDTH) ? FONT_8x8 : boldFontBestFit;
        int col = centerTextHorizontal(font, textlen);
        if (row < 0)
            row = centerTextVertical(font);
        oledWriteString(&ssoled, 0, col, row, (char *)text, font, !color, 0);
    }
}

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

bool ui::begin(int pixels_width, int pixels_height, displayType_t displayType, bool flipUpsideDown)
{
    SCREEN_WIDTH = pixels_width;
    SCREEN_HEIGHT = pixels_height;
    int angle = (flipUpsideDown) ? ANGLE_180 : ANGLE_0;

    int result = oledInit(
        &ssoled,
        displayType,
        -1,
        angle,
        0, 0,
        SCREEN_SDA_PIN, SCREEN_SCL_PIN,
        SCREEN_RESET_PIN, 400000L);
    if (result == OLED_NOT_FOUND)
    {
        // YOU HAVE TO DO THIS TWICE. DON'T ASK ME WHY.
        result = oledInit(
            &ssoled,
            displayType,
            -1,
            angle,
            0, 0,
            SCREEN_SDA_PIN, SCREEN_SCL_PIN,
            SCREEN_RESET_PIN, 400000L);
        available = (result != OLED_NOT_FOUND);
    }
    else
        available = true;

    if (available)
    {
        oledPower(&ssoled, true);
        oledSetTextWrap(&ssoled, false);
        oledFill(&ssoled, 0, 1);

        switch (displayType)
        {
        case SSOLED_128x128:
            boldFontBestFit = FONT_16x32;
            break;
        case SSOLED_128x32:
            boldFontBestFit = FONT_16x16;
            break;
        case SSOLED_128x64:
            boldFontBestFit = FONT_16x32;
            break;
        case SSOLED_132x64:
            boldFontBestFit = FONT_16x32;
            break;
        default:
            boldFontBestFit = FONT_8x8;
            break;
        }

        oledSetBackBuffer(&ssoled, uiManager::getFrameServerBuffer());
        oledFill(&ssoled, 0, 0);
        drawTextBestFit((const char *)str_wellcome, 2);
        oledDumpBuffer(&ssoled, nullptr);
        vTaskDelay(2000 / portTICK_RATE_MS);
        oledFill(&ssoled, 0, 0);
        oledDumpBuffer(&ssoled, nullptr);
        oledSetBackBuffer(&ssoled, nullptr);

        Preferences prefs;
        if (prefs.begin(PREFS_NAMESPACE, true))
        {
            bool en = prefs.getBool(KEY_FRAMESERVER, false);
            prefs.end();
            frameServerInternalSetEnabled(en);
        }
    }
    return available;
}

// ----------------------------------------------------------------------------
// Display
// ----------------------------------------------------------------------------

void ui::clear()
{
    if (available)
    {
        oledFill(&ssoled, 0, 1);
    }
}

void ui::display(uint8_t *buffer)
{
    oledDumpBuffer(&ssoled, buffer);
}

void ui::turnOff()
{
    if (available)
        oledPower(&ssoled, false);
}

// ----------------------------------------------------------------------------
// Paint
// ----------------------------------------------------------------------------

void ui::showBitePoint(clutchValue_t value)
{
    if (!available)
        return;
    uint8_t *buffer = enterDisplay(SCR_MENU_PRIORITY);
    oledSetBackBuffer(&ssoled, buffer);
    int iPercent = (value - CLUTCH_NONE_VALUE) * 100 / (CLUTCH_FULL_VALUE - CLUTCH_NONE_VALUE);
    char sPercent[6];
    snprintf(sPercent, 6, "% 3d%%", iPercent);
    oledFill(&ssoled, 0, 0);
    drawTitle(str_clutch_cal);
    drawTextBestFit(sPercent);
    drawScaleBar(SCREEN_HEIGHT - 8, iPercent, 0, 100);
    exitDisplay(SCR_MENU_PRIORITY, true);
}

void ui::showMenu(const char *title, const char *selection)
{
    if (!available)
        return;
    uint8_t *buffer = enterDisplay(SCR_MENU_PRIORITY);
    oledSetBackBuffer(&ssoled, buffer);
    oledFill(&ssoled, 0, 0);
    drawTitle(title);
    drawTextBestFit(selection, 2);
    exitDisplay(SCR_MENU_PRIORITY, false);
}

void ui::hideMenu()
{
    if (available)
        uiManager::hide(SCR_MENU_PRIORITY);
}

void ui::showInfo(const char *title, const char *info, screenPriority_t priority)
{
    if (!available)
        return;
    uint8_t *buffer = enterDisplay(priority);
    oledSetBackBuffer(&ssoled, buffer);
    oledFill(&ssoled, 0, 0);
    oledRectangle(&ssoled, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, 1, 0);
    oledRectangle(&ssoled, 1, 1, SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2, 1, 0);
    if (title != nullptr)
        drawTitle(title);
    drawTextBestFit(info);
    exitDisplay(priority, true);
}

void ui::showModal(const char *title, const char *info, screenPriority_t priority)
{
    if (!available)
        return;
    for (int i = 0; i < 12; i++)
    {
        uint8_t *buffer = enterDisplay(priority);
        oledSetBackBuffer(&ssoled, buffer);
        oledFill(&ssoled, 0, 0);
        if (i % 2)
        {
            oledRectangle(&ssoled, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, 1, 0);
            oledRectangle(&ssoled, 1, 1, SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2, 1, 0);
        }
        if (title != nullptr)
            drawTitle(title);
        drawTextBestFit(info);
        exitDisplay(priority, false);
        vTaskDelay(MODAL_BLINK_RATE_TICKS);
    }
    hide(priority);
}

// ----------------------------------------------------------------------------
// Paint macros
// ----------------------------------------------------------------------------

void ui::showSaveNote()
{
    ui::showInfo(nullptr, str_saved, SCR_MENU_PRIORITY);
}

void ui::showConnectedNotice()
{
    ui::showInfo(nullptr, str_connected, SCR_INFO_PRIORITY);
}

void ui::showBLEDiscoveringNotice()
{
    ui::showInfo(nullptr, (const char *)"(((.)))", SCR_INFO_PRIORITY);
}

void ui::showLowBatteryNotice()
{
    if (!available)
        return;
    uint8_t *buffer = enterDisplay(SCR_INFO_PRIORITY);
    int left = (SCREEN_WIDTH - BATTERY_LOW_ICON_WIDTH) / 2;
    int top = (SCREEN_HEIGHT - BATTERY_LOW_ICON_HEIGHT) / 2;
    oledSetBackBuffer(&ssoled, buffer);
    oledFill(&ssoled, 0, 0);

    oledRectangle(&ssoled, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, 1, 0);
    oledRectangle(&ssoled, 1, 1, SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2, 1, 0);
    oledRectangle(&ssoled, left, top, left + BATTERY_LOW_ICON_WIDTH, top + BATTERY_LOW_ICON_HEIGHT, 0, 1);
    drawTitle(str_battery);

    oledRectangle(&ssoled, left, top, left + BATTERY_LOW_ICON_WIDTH, top + BATTERY_LOW_ICON_HEIGHT, 0, 1);
    oledRectangle(&ssoled, left + 4, top + 7, left + 31, top + 21, 1, 0);
    oledRectangle(&ssoled, left, top + 9, left + 4, top + 18, 1, 0);
    oledSetPixel(&ssoled, left + 1, top + 10, 1, 0);
    oledSetPixel(&ssoled, left + 1, top + 17, 1, 0);
    oledDrawLine(&ssoled, left + 29, top + 10, left + 29, top + 18, 0);
    oledDrawLine(&ssoled, left + 28, top + 12, left + 28, top + 18, 0);
    oledDrawLine(&ssoled, left + 27, top + 14, left + 27, top + 18, 0);
    oledDrawLine(&ssoled, left + 26, top + 16, left + 26, top + 18, 0);
    oledSetPixel(&ssoled, left + 25, top + 18, 1, 0);
    oledDrawLine(&ssoled, left + 23, top + 1, left + 11, top + 25, 0);
    oledDrawLine(&ssoled, left + 24, top + 1, left + 11, top + 26, 0);
    oledDrawLine(&ssoled, left + 23, top + 2, left + 12, top + 25, 0);
    exitDisplay(SCR_INFO_PRIORITY, false);
}

// ----------------------------------------------------------------------------
// Other
// ----------------------------------------------------------------------------

bool ui::isAvailable()
{
    return available;
}

// ----------------------------------------------------------------------------
// Frame server
// ----------------------------------------------------------------------------

void frameServerLoop(void *unused)
{
    uint8_t *buffer;
    SSOLED frameServerOled = ssoled;
    TickType_t rate;
    char text[7];

    // Initialize
    buffer = uiManager::getFrameServerBuffer();
    oledSetBackBuffer(&frameServerOled, buffer);
    rate = xTaskGetTickCount();

    // coordinates
    // int x_speed = (SCREEN_WIDTH - (fontWidth[FONT_8x8] * 3)) / 2;
    int x_right = SCREEN_WIDTH - (fontWidth[FONT_8x8] * 6);
    int y_bottom = (SCREEN_HEIGHT - (fontHeight[FONT_8x8])) / 8;

    while (true)
    {
        if (frameServerEnabled)
        {
            oledFill(&frameServerOled, 0, 0);
            if (uartServer::gear > ' ')
            {
                // draw speed
                snprintf(text, 7, "Sp:%3.3d", uartServer::speed);
                oledWriteString(&frameServerOled, 0,
                                x_right,
                                0, // top
                                (char *)text, FONT_8x8, 1, 0);

                // draw engine map
                snprintf(text, 7, "Map:%2.2d", uartServer::engineMap);
                oledWriteString(&frameServerOled, 0,
                                0, // left
                                0, // top
                                (char *)text, FONT_8x8, 0, 0);

                // draw abs level
                snprintf(text, 7, "ABS:%2.2d", uartServer::absLevel);
                oledWriteString(&frameServerOled, 0,
                                0, // left
                                y_bottom,
                                (char *)text, FONT_8x8, 0, 0);

                // draw tclevel
                snprintf(text, 7, " TC:%2.2d", uartServer::tcLevel);
                oledWriteString(&frameServerOled, 0,
                                x_right,
                                y_bottom,
                                (char *)text, FONT_8x8, 0, 0);

                // draw current gear and RPM limit warning
                if (uartServer::rpmPercent > 98)
                {
                    snprintf(text, 7, "[%c]", uartServer::gear);
                    oledWriteString(&frameServerOled, 0,
                                    centerTextHorizontal(boldFontBestFit, 3),
                                    centerTextVertical(boldFontBestFit),
                                    (char *)text, boldFontBestFit, 0, 0);
                }
                else
                {
                    snprintf(text, 7, "%c", uartServer::gear);
                    oledWriteString(&frameServerOled, 0,
                                    centerTextHorizontal(boldFontBestFit, 1),
                                    centerTextVertical(boldFontBestFit),
                                    (char *)text, boldFontBestFit, 0, 0);
                }
            }
            // Display frame
            uiManager::unsafeDisplayFrameServerBuffer();
            // Adapt to target FPS
            vTaskDelayUntil(&rate, targetFrameTime);
            // if (!xTaskDelayUntil(&rate, targetFrameTime))
            //     // avoid CPU starvation
            //     vTaskDelay(11/portTICK_RATE_MS);
        }
        else
        {
            // clear buffer before disable
            uiManager::hide(SCR_FRAMESERVER_PRIORITY);

            // wait for enable
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            // Initialize again
            buffer = uiManager::getFrameServerBuffer();
            oledSetBackBuffer(&frameServerOled, buffer);
            rate = xTaskGetTickCount();
        }
    }
}

void frameServerInternalSetEnabled(bool state)
{
    if (available)
    {
        if ((frameServerDaemon == nullptr) && state)
        {
            xTaskCreate(
                frameServerLoop,
                "FrameSrv",
                FRAMESERVER_STACK_SIZE,
                nullptr,
                tskIDLE_PRIORITY,
                // UART_TASK_PRIORITY, // Do not work
                &frameServerDaemon);
        }
        if (frameServerDaemon && (state != frameServerEnabled))
        {
            frameServerEnabled = state;
            if (state)
                xTaskNotifyGive(frameServerDaemon); // signal
        }
    }
}

void ui::frameServerSetEnabled(bool state)
{
    frameServerInternalSetEnabled(state);
    Preferences prefs;
    if (prefs.begin(PREFS_NAMESPACE, false))
    {
        prefs.putBool(KEY_FRAMESERVER, state);
        prefs.end();
    }
}