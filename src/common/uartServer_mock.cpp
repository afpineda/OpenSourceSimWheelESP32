/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Dummy implementation of the `uartServer` namespace
 * 
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 * 
 */

#include "SimWheel.h"

volatile char uartServer::gear = ' ';
volatile uint8_t uartServer::rpmPercent = 0;
volatile uint16_t uartServer::speed = 0;
volatile uint8_t uartServer::engineMap = 0;
volatile uint8_t uartServer::absLevel = 0;
volatile uint8_t uartServer::tcLevel = 0;
volatile uint64_t uartServer::frameCount = 0;

void uartServer::onReceive(char *text)
{
    
}