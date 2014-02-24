#ifndef CTIMER_H
#define CTIMER_H

#include <arduino.h>
#include <stdarg.h>
#include "cmodule.h"

class CTimer :CModule
{
    public:
        CTimer();
        virtual ~CTimer();

        virtual void Update();
        virtual void Setup();

        virtual void ReadTime();

        void PrintTime();

        short GetHour();
        short GetMinute();
        short GetSecond();
        short GetDayOfMonth();

    protected:
        void SetTime();
        uint8_t decToBcd(byte b);
        uint8_t bcdToDec(byte b);

// Initializes all values:
        byte m_nSecond;
        byte m_nMinute;
        byte m_nHour;

    private:
        unsigned long m_lPreviousMillis;
        unsigned long m_lPreviousSecondUpdateMillis;


        byte m_nWeekday;
        byte m_nMonthday;
        byte m_nMonth;
        byte m_nYear;

        byte ReadByte();


};

#endif // CTIMER_H
