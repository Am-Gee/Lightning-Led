#ifndef CFANCONTROLLER_H
#define CFANCONTROLLER_H

#include "executor.h"
#include "tempsense.h"
#include "onewiretempsensor.h"
#include "csensor.h"

class CFanController : public CSensor, public Executor
{
    public:
        CFanController(int id, OneWire* pDS, byte* pSensorAdress, byte* pSensorAdress2, byte* pSensorAdress3, byte* pAdr20W1);
        virtual ~CFanController();

        virtual void Setup();
        virtual void OnUpdate();

        virtual void SetValue(int value, float fValue);
        virtual float GetValue(int value);

        void SetSpeed(int speed);

        virtual float GetSensorValue();
        virtual long Execute(int method, int params, char* pfParamArr[10], bool& writeAnswer);

    protected:
    private:
        byte m_iLastPercent;
        int m_iCurrentSpeed;
        unsigned long m_lLastMillis;
        float m_fLastMaxTemp;

        COneWireTempSensor*  m_pTempSensorDS;
        COneWireTempSensor*  m_pTempSensorDS2;
        COneWireTempSensor*  m_pTempSensorDS3;

        COneWireTempSensor*  m_pSensor20Watt;

};

#endif // CFANCONTROLLER_H
