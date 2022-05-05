/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Unit Test. See [README](./README.md)
 * 
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 * 
 */

#include <Arduino.h>
#include <stdarg.h>
#include <string.h>
#include "SimWheel.h"
#include "debugUtils.h"

//------------------------------------------------------------------
// Globals
//------------------------------------------------------------------

char response_error[] = "\r\nERROR\r\n";
char response_ok[] = "\r\nOK\r\n";
#define MAX_UART_OUTPUT_COUNT 20
char uartOutput[MAX_UART_OUTPUT_COUNT][80];
int uartOutputCount = 0;

//------------------------------------------------------------------
// Mocks
//------------------------------------------------------------------

bool hidImplementation::uartSendText(char *text)
{
    if (uartOutputCount < MAX_UART_OUTPUT_COUNT)
        strcpy(uartOutput[uartOutputCount++], text);
    else
        abort();
    return true;
}

void batteryCalibration::restartAutoCalibration()
{
    
}

//------------------------------------------------------------------
// Auxiliary
//------------------------------------------------------------------

void resetUartOutput()
{
    uartOutputCount = 0;
}

void resetData()
{
    uartServer::gear = ' ';
    uartServer::rpmPercent = 0;
    uartServer::speed = 0;
    uartServer::engineMap = 0;
    uartServer::absLevel = 0;
    uartServer::tcLevel = 0;
}

void assertOutputEquals(int index, char *text)
{
    if ((index < 0) || (index >= MAX_UART_OUTPUT_COUNT))
        abort();
    if (strcmp(uartOutput[index], text) != 0)
    {
        Serial.println("UART OUTPUT MISTMATCH (expected --- obtained ---)");
        Serial.println(text);
        Serial.println("----------------");
        Serial.println(uartOutput[index]);
        Serial.println("----------------");
    }
}

void assertOutputCountEquals(int count)
{
    if (count != uartOutputCount)
    {
        Serial.println("UART OUTPUT LENGTH MISTMATCH");
        serialPrintf("EXPECTED %d, OBTAINED %d\n", count, uartOutputCount);
    }
}

void assertDataEquals(
    char gear = ' ',
    uint8_t rpmPercent = 0,
    uint16_t speed = 0,
    uint8_t engineMap = 0,
    uint8_t absLevel = 0,
    uint8_t tcLevel = 0)
{
    if ((uartServer::gear != gear) || (uartServer::rpmPercent != rpmPercent) ||
        (uartServer::speed != speed) || (uartServer::engineMap != engineMap) ||
        (uartServer::absLevel != absLevel) || (uartServer::tcLevel != tcLevel))
    {
        Serial.println("DATA MISTMATCH (expected:obtained)");
        serialPrintf("GEAR      =(%c:%c)\n", uartServer::gear, gear);
        serialPrintf("SPEED     =(%d:%d)\n", uartServer::speed, speed);
        serialPrintf("ENGINEMAP =(%d:%d)\n", uartServer::engineMap, engineMap);
        serialPrintf("ABSLEVEL  =(%d:%d)\n", uartServer::absLevel, absLevel);
        serialPrintf("TCLEVEL   =(%d:%d)\n", uartServer::tcLevel, tcLevel);
    }
}

//------------------------------------------------------------------
// Arduino entry point
//------------------------------------------------------------------

void setup()
{
    int testCounter = 1;
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("--GO--");

    // ----- Test AT commands

    // --- Positive tests

    printTestHeader(testCounter++); // #1
    resetUartOutput();
    uartServer::onReceive((char*)"AT+C=0xFF");
    assertOutputEquals(0, response_ok);
    assertOutputCountEquals(1);

    printTestHeader(testCounter++);
    resetUartOutput();
    uartServer::onReceive((char*)"AT+C?");
    assertOutputEquals(0, (char *)"\r\nFF\r\n");
    assertOutputEquals(1, response_ok);
    assertOutputCountEquals(2);

    printTestHeader(testCounter++);
    resetUartOutput();
    uartServer::onReceive((char*)"AT+C=-127;");
    assertOutputEquals(0, response_ok);
    assertOutputCountEquals(1);

    printTestHeader(testCounter++);
    resetUartOutput();
    uartServer::onReceive((char*)"AT+C?;;;");
    assertOutputEquals(0, (char*)"\r\n81\r\n");
    assertOutputEquals(1, (char*)response_ok);
    assertOutputCountEquals(2);

    printTestHeader(testCounter++);
    resetUartOutput();
    uartServer::onReceive((char*)"AT+A=1;;;+A?");
    assertOutputEquals(0, (char*)response_ok);
    assertOutputEquals(1, (char*)"\r\n1\r\n");
    assertOutputEquals(2, (char*)response_ok);
    assertOutputCountEquals(3);

    printTestHeader(testCounter++);
    resetUartOutput();
    uartServer::onReceive((char*)"AT+B=y;;;+B?");
    assertOutputEquals(0, (char*)response_ok);
    assertOutputEquals(1, (char*)"\r\nY\r\n");
    assertOutputEquals(2, (char*)response_ok);
    assertOutputCountEquals(3);

    printTestHeader(testCounter++);
    resetUartOutput();
    uartServer::onReceive((char*)"AT+B=N;;;+B?");
    assertOutputEquals(0, (char*)response_ok);
    assertOutputEquals(1, (char*)"\r\nN\r\n");
    assertOutputEquals(2, (char*)response_ok);
    assertOutputCountEquals(3);

    printTestHeader(testCounter++);
    resetUartOutput();
    uartServer::onReceive((char*)"AT+R=Y;;;+R=y");
    assertOutputEquals(0, (char*)response_ok);
    assertOutputEquals(1, (char*)response_ok);
    assertOutputCountEquals(2);

    // --- Negative tests

    printTestHeader(testCounter++); // #9
    resetUartOutput();
    uartServer::onReceive((char*)"AT");
    assertOutputEquals(0, (char*)response_error);
    assertOutputCountEquals(1);

    printTestHeader(testCounter++);
    resetUartOutput();
    uartServer::onReceive((char*)"AT;");
    assertOutputEquals(0, (char*)response_error);
    assertOutputCountEquals(1);

    printTestHeader(testCounter++);
    resetUartOutput();
    uartServer::onReceive((char*)"AT+CC?;");
    assertOutputEquals(0, (char*)response_error);
    assertOutputCountEquals(1);

    printTestHeader(testCounter++);
    resetUartOutput();
    uartServer::onReceive((char *)"AT+CC?;+C;");
    assertOutputEquals(0, (char *)response_error);
    assertOutputEquals(1, (char *)response_error);
    assertOutputCountEquals(2);

    printTestHeader(testCounter++);
    resetUartOutput();
    uartServer::onReceive((char*)"AT+A=7");
    assertOutputEquals(0, response_error);
    assertOutputCountEquals(1);

    printTestHeader(testCounter++);
    resetUartOutput();
    uartServer::onReceive((char*)"AT+B=1");
    assertOutputEquals(0, response_error);
    assertOutputCountEquals(1);

    printTestHeader(testCounter++);
    resetUartOutput();
    uartServer::onReceive((char*)"AT+B=");
    assertOutputEquals(0, response_error);
    assertOutputCountEquals(1);

    printTestHeader(testCounter++);
    resetUartOutput();
    uartServer::onReceive((char*)"AT+B=;");
    assertOutputEquals(0, response_error);
    assertOutputCountEquals(1);

    printTestHeader(testCounter++);
    resetUartOutput();
    uartServer::onReceive((char*)"AT+R=1");
    assertOutputEquals(0, response_error);
    assertOutputCountEquals(1);


    // ----- Test frameserver data

    resetData();
    printTestHeader(testCounter++); // #18
    uartServer::onReceive((char*)"!N552D01050403");
    assertDataEquals('N',85,301,5,4,3);

    printTestHeader(testCounter++);
    uartServer::onReceive((char*)"!");
    assertDataEquals((char)0,0,0,0,0,0);

    printTestHeader(testCounter++);
    uartServer::onReceive((char*)"!P");
    assertDataEquals('P',0,0,0,0,0);

    printTestHeader(testCounter++);
    uartServer::onReceive((char*)"!100");
    assertDataEquals('1',0,0,0,0,0);

    printTestHeader(testCounter++);
    uartServer::onReceive((char*)"!2647D00");
    assertDataEquals('2',100,125,0,0,0);

    printTestHeader(testCounter++);
    uartServer::onReceive((char*)"!30001000A");
    assertDataEquals('3',0,1,10,0,0);

    printTestHeader(testCounter++);
    uartServer::onReceive((char*)"!40102000702");
    assertDataEquals('4',1,2,7,2,0);

    printTestHeader(testCounter++); 
    uartServer::onReceive((char*)"!N552D0105040301234567");
    assertDataEquals('N',85,301,5,4,3);

    // ----- Test unknown input

    resetData();
    printTestHeader(testCounter++); // #26
    resetUartOutput();
    uartServer::onReceive((char*)"xxyyxx;xxzz");
    assertOutputCountEquals(0);
    assertDataEquals();

    printTestHeader(testCounter++);
    resetUartOutput();
    uartServer::onReceive((char*)"at+c?;"); // Note: not a valid AT command
    assertOutputCountEquals(0);
    assertDataEquals();

    printTestHeader(testCounter++);
    resetUartOutput();
    uartServer::onReceive((char*)"\n");
    assertOutputCountEquals(0);
    assertDataEquals();


    // ----- END
    Serial.println("--END (reset to restart) --");
    while (true)
        ;
}

void loop()
{
}