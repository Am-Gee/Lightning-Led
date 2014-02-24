#ifndef SIMULATEDCLOCK_H
#define SIMULATEDCLOCK_H

#include "timer.h"

class CSimulatedClock : public CTimer
{
    public:
        CSimulatedClock();
        virtual ~CSimulatedClock();

        void SetTime(short hour, short minute, short second);

        virtual void Setup();
        virtual void Update();
        virtual void ReadTime();

    protected:
    private:
        unsigned long m_lPreviousMillis;

};

#endif // SIMULATEDCLOCK_H
