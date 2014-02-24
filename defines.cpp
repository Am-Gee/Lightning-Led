#include <Arduino.h>
#include "defines.h"

void printDebugString(char *fmt, ... )
{
    char debugString[64]; // resulting string limited to 128 chars
    va_list args;
    va_start (args, fmt );
    vsnprintf(debugString, 128, fmt, args);
    va_end (args);
    Serial.println(debugString);
}
