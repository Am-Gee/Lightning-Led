#ifndef CTEMPSENSE_H
#define CTEMPSENSE_H
#include <Arduino.h>

#include "csensor.h"

class CTempSensor : public CSensor
{
    public:
        CTempSensor(uint8_t inputPin, long lUpdateInterval);
        virtual ~CTempSensor();

        virtual void Setup();
        virtual void OnUpdate();
        virtual float GetSensorValue();

        uint8_t m_nInputPin;

    protected:


    private:
        float m_fValue;

};

#endif // CTEMPSENSE_H
