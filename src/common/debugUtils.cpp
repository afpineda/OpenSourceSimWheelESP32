/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2022-02-27
 * @brief Testing and debugging utilities
 * 
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 * 
 */

#include "debugUtils.h"
#include <Arduino.h>

void debugPrintBool(inputBitmap_t state)
{
    for (int i=((sizeof(inputBitmap_t)*8)-1); i>=0; i--) {
        if (BITMAP(i) & state)
            Serial.print("1");
        else
            Serial.print("0");
    }
}

void printTestHeader(int index)
{
    Serial.print("**TEST #");
    Serial.println(index);
}

void serialPrintf(const char *fmt, ...)
{
    char buff[257];
    va_list pargs;
    va_start(pargs, fmt);
    vsnprintf(buff, 256, fmt, pargs);
    va_end(pargs);
    buff[256] = 0;
    Serial.print(buff);
}