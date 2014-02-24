#ifndef DEFINES_H
#define DEFINES_H

#include <Arduino.h>
#include <EEPROM.h>

#define SIMULATE_TIME_LAPSE 1 // 1 = normal time 30 means 1 second is 30 seconds
#define UPDATE_INTERVAL 500UL // ms

void printDebugString(char *fmt, ... );

struct DimTime
{
    int hour;
    int minute;
};

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          EEPROM.write(ee++, *p++);
    return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
    {
        byte bRead = EEPROM.read(ee++);
        *p++ = bRead;
    }
    return i;
}

#endif // DEFINES_H
