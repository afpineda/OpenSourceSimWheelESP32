/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Implementation of the `uartServer` namespace
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "SimWheel.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

using namespace hidImplementation;

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

#define COMMAND_MAX_LENGTH 81
static const char response_error[] = "\r\nERROR\r\n";
static const char response_ok[] = "\r\nOK\r\n";

volatile char uartServer::gear = ' ';
volatile uint8_t uartServer::rpmPercent = 0;
volatile uint16_t uartServer::speed = 0;
volatile uint8_t uartServer::engineMap = 0;
volatile uint8_t uartServer::absLevel = 0;
volatile uint8_t uartServer::tcLevel = 0;
volatile uint64_t uartServer::frameCount = 0;

// ----------------------------------------------------------------------------
// auxiliary
// ----------------------------------------------------------------------------

inline bool isTerminatorString(char *text)
{
    return (text[0] == 0) || (text[0] == '\n') || (text[0] == '\r');
}

inline char *getSeparator(char *text)
{
    char *separator;
    separator = strchr(text, ';');
    if (!separator)
        separator = strchr(text, '\r');
    if (!separator)
        separator = strchr(text, '\n');
    if (!separator)
        separator = strchr(text, 0);
    return separator;
}

char *readByte(char *text, uint8_t *dest)
{
    uint8_t byte;
    if ((text[0] >= '0') && (text[0] <= '9'))
        byte = ((uint8_t)(text[0] - '0')) << 4;
    else if ((text[0] >= 'A') && (text[0] <= 'F'))
        byte = ((uint8_t)(text[0]) - 55) << 4;
    else
        return nullptr;

    if ((text[1] >= '0') && (text[1] <= '9'))
        byte |= ((uint8_t)(text[1] - '0'));
    else if ((text[1] >= 'A') && (text[1] <= 'F'))
        byte |= ((uint8_t)(text[1]) - 55);
    else
        return nullptr;

    *dest = byte;
    return text + 2;
}

// ----------------------------------------------------------------------------
// Receive data and parse
// ----------------------------------------------------------------------------

void uartServer::onReceive(char *text)
{
    // Serial.print("onReceive: ");
    // Serial.println(text);
    // Serial.print("Len; ");
    // Serial.println(strlen(text));

    // PARSE AT COMMANDS
    if ((text[0] == 'A') && (text[1] == 'T'))
    {
        text = text + 2;
        if ((text[0] == ';') || isTerminatorString(text))
        {
            uartSendText((char *)response_error);
            return;
        }
        char commandData[COMMAND_MAX_LENGTH];

        while (!isTerminatorString(text))
        {
            // ignore empty commands
            while (text[0] == ';')
                text = text + 1;

            char *separator = getSeparator(text);

            if ((text[0] == '+') && (text[1] >= 65) && (text[1] <= 90) && ((text[2] == '?') || text[2] == '='))
            {
                bool setOrGet = (text[2] == '=');
                if (setOrGet)
                {
                    int dataLength = (int)(separator - text) - 3;
                    if ((dataLength > 0) && (dataLength < COMMAND_MAX_LENGTH))
                    {
                        memset(commandData, 0, COMMAND_MAX_LENGTH);
                        strncpy(commandData, text + 3, dataLength);
                        // Serial.print("DATA: ");
                        // Serial.println(commandData);
                    }
                    else
                    {
                        uartSendText((char *)response_error);
                        text = separator;
                        continue;
                    }
                }

                switch (text[1])
                {
                case 'A':
                    // COMMAND: Clutch paddle function
                    if (setOrGet)
                    {
                        errno = 0;
                        int value = strtol(commandData, nullptr, 0);
                        // Serial.print("UART: CLUTCH FUNCTION ");
                        // Serial.println(value);
                        if ((errno == 0) && (value >= CF_CLUTCH) && (value <= CF_BUTTON))
                        {
                            inputHub::setClutchFunction((clutchFunction_t)value, true);
                            uartSendText((char *)response_ok);
                        }
                        else
                            uartSendText((char *)response_error);
                    }
                    else
                    {
                        clutchValue_t value = inputHub::getClutchFunction();
                        snprintf(commandData, COMMAND_MAX_LENGTH, "\r\n%1d\r\n", value);
                        uartSendText(commandData);
                        uartSendText((char *)response_ok);
                    }
                    break;

                case 'B':
                    // COMMAND: ALT buttons function
                    if (setOrGet)
                    {
                        bool cmdyes = (commandData[0] == 'Y') || (commandData[0] == 'y');
                        bool cmdno = (commandData[0] == 'N') || (commandData[0] == 'n');
                        if ((commandData[1] == 0) && (cmdyes || cmdno))
                        {
                            inputHub::setALTFunction(cmdyes, true);
                            uartSendText((char *)response_ok);
                        }
                        else
                            uartSendText((char *)response_error);
                    }
                    else
                    {
                        if (inputHub::getALTFunction())
                            uartSendText((char *)"\r\nY\r\n");
                        else
                            uartSendText((char *)"\r\nN\r\n");
                        uartSendText((char *)response_ok);
                    }
                    break;

                case 'C':
                    // COMMAND: Clutch bite point
                    if (setOrGet)
                    {
                        errno = 0;
                        clutchValue_t value = strtol(commandData, nullptr, 0);
                        // Serial.print("UART: BITE POINT ");
                        // Serial.println(value);
                        if ((errno == 0) && (value >= CLUTCH_NONE_VALUE) && (value <= CLUTCH_FULL_VALUE))
                        {
                            inputHub::setClutchBitePoint(value, true);
                            uartSendText((char *)response_ok);
                        }
                        else
                            uartSendText((char *)response_error);
                    }
                    else
                    {
                        clutchValue_t value = inputHub::getClutchBitePoint();
                        snprintf(commandData, COMMAND_MAX_LENGTH, "\r\n%02X\r\n", (uint8_t)value);
                        uartSendText(commandData);
                        uartSendText((char *)response_ok);
                    }
                    break;

                case 'R':
                    // COMMAND: Restart battery auto-calibration
                    if (setOrGet && ((commandData[0] == 'Y') || (commandData[0] == 'y')) && (commandData[1] == 0))
                    {
                        batteryCalibration::restartAutoCalibration();
                        uartSendText((char *)response_ok);
                    }
                    else
                    {
                        uartSendText((char *)response_error);
                    }
                    break;

                default:
                    uartSendText((char *)response_error);
                    break;
                }
            }
            else if (!isTerminatorString(text) && (text[0] != ';'))
            {
                uartSendText((char *)response_error);
            }

            // Next loop
            text = separator;
        } // end while
    }
    else if (text[0] == '!') // PARSE FRAMESERVER DATA
    {
        // update simulator data
        register uint16_t speed;
        text++;
        uartServer::frameCount++;
        if (text[0] < 32)
        {
            uartServer::gear = (char)0;
            uartServer::engineMap = 0;
            uartServer::absLevel = 0;
            uartServer::tcLevel = 0;
            uartServer::speed = 0;
            uartServer::rpmPercent = 0;
            return;
        }
        uartServer::gear = text[0];
        text++;
        text = readByte(text, (uint8_t *)&uartServer::rpmPercent);
        if (text)
        {
            speed = 0;
            text = readByte(text, (uint8_t *)(&speed));
            if (text)
            {
                text = readByte(text, (uint8_t *)(&speed) + 1);
                uartServer::speed = speed;
                if (text)
                {
                    text = readByte(text, (uint8_t *)(&uartServer::engineMap));
                    if (text)
                    {
                        text = readByte(text, (uint8_t *)(&uartServer::absLevel));
                        if (text)
                        {
                            if (!readByte(text, (uint8_t *)(&uartServer::tcLevel)))
                                uartServer::tcLevel = 0;
                        }
                        else
                        {
                            uartServer::tcLevel = 0;
                            uartServer::absLevel = 0;
                        }
                    }
                    else
                    {
                        uartServer::absLevel = 0;
                        uartServer::tcLevel = 0;
                        uartServer::engineMap = 0;
                    }
                }
                else
                {
                    uartServer::absLevel = 0;
                    uartServer::tcLevel = 0;
                    uartServer::engineMap = 0;
                    uartServer::speed = 0; 
                }
            }
            else
            {
                uartServer::engineMap = 0;
                uartServer::absLevel = 0;
                uartServer::tcLevel = 0;
                uartServer::speed = 0;
            }
        }
        else
        {
            uartServer::engineMap = 0;
            uartServer::absLevel = 0;
            uartServer::tcLevel = 0;
            uartServer::speed = 0;
            uartServer::rpmPercent = 0;
        }
    }
}
