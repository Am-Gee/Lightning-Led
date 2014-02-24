#ifndef RELAYCONTROLLER_H
#define RELAYCONTROLLER_H
#include <arduino.h>

#include "cmodule.h"
#include "executor.h"


class RelayController : public Executor, public CModule
{
    public:
        RelayController();
        virtual ~RelayController();

        virtual void Setup();
        virtual void Update();

        virtual void SetValue(int value, float fValue);
        virtual float GetValue(int value);
        virtual long Execute(int method, int params, char* pfParamArr[10], bool& writeAnswer);

        void SetRelayState(int relayNum, bool turnOn);

    protected:
    private:
        bool m_bPinStateChanged;
};

#endif // RELAYCONTROLLER_H
