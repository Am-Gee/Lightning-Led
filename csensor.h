#ifndef CSENSOR_H
#define CSENSOR_H

#include "csensor.h"
#include "cmodule.h"


class CSensor : public CModule
{
    public:
        CSensor(unsigned long lUpdateInterval);
        virtual ~CSensor();

        virtual float GetSensorValue() = 0;

    protected:
    private:
};

#endif // CSENSOR_H
