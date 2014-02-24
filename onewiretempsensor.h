#ifndef ONEWIRETEMPSENSOR_H
#define ONEWIRETEMPSENSOR_H

#include <Arduino.h>
#include <OneWire.h>
#include "csensor.h"

class COneWireTempSensor : public CSensor
{
    public:
        COneWireTempSensor(int id, OneWire* pDS, byte* adress, long interval);
        virtual ~COneWireTempSensor();

        virtual void Setup();
        virtual void OnUpdate();
        virtual float GetSensorValue();
        int GetId();

    protected:
        void WriteTimeToScratchpad();
        float ReadTimeFromScratchpad(byte* data);
    private:
        uint8_t fastReset(void);
        float m_fLastValue;

        int m_id;
        byte* m_arrSensorAdress;
        OneWire* m_pDS;
        bool m_bWrite;

        short mode;

        IO_REG_TYPE bitmask;
        volatile IO_REG_TYPE *baseReg;


};

#endif // ONEWIRETEMPSENSOR_H
