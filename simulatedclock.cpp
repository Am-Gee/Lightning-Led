#include <Arduino.h>
#include "defines.h"
#include "timer.h"
#include "csimulatedclock.h"

CSimulatedClock::CSimulatedClock() : CTimer()
{
    CTimer::m_nHour = 0;
    CTimer::m_nMinute = 0;
    CTimer::m_nSecond = 0;
    m_lPreviousMillis = millis();
}

CSimulatedClock::~CSimulatedClock()
{
    //dtor
}

void CSimulatedClock::SetTime(short hour, short minute, short second)
{
    CTimer::m_nHour = hour;
    CTimer::m_nMinute = minute;
    CTimer::m_nSecond = second;
}

void CSimulatedClock::Setup()
{
    CTimer::m_nHour = 8;
    CTimer::m_nMinute = 0;
    CTimer::m_nSecond = 0;

    CTimer::Setup();
}

void CSimulatedClock::Update()
{
    unsigned long currentMillis = millis();

    if(currentMillis - m_lPreviousMillis > 1000)
    {
        m_lPreviousMillis = currentMillis;

        CTimer::m_nSecond += SIMULATE_TIME_LAPSE;

        if(CTimer::m_nSecond >= 60)
        {
            CTimer::m_nMinute++;
            CTimer::m_nSecond = 0;

            PrintTime();
        }
        if(CTimer::m_nMinute >= 60)
        {
            CTimer::m_nHour++;
            CTimer::m_nMinute = 0;
            CTimer::m_nSecond = 0;
        }
        if(CTimer::m_nHour >= 24)
        {
            CTimer::m_nHour = 0;
            CTimer::m_nMinute = 0;
            CTimer::m_nSecond = 0;
        }
    }
}

void CSimulatedClock::ReadTime()
{
    // do nothing.
}
