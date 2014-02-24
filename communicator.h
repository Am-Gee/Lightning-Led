#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include "executor.h"

#define MAX_CMD_LEN 32

class Communicator : public CModule, Executor
{

    enum Mode
        {
            WRITE,
            READ,
            EXECUTE
        };

    enum IO_Device
        {
            SERIAL_PORT0,
            SERIAL_PORT1

        };

    public:
        Communicator();
        virtual ~Communicator();

        virtual void Setup();

        virtual void Update();

        virtual void SetValue(int value, float fValue);
        virtual float GetValue(int value);
        virtual long Execute(int method, int params, char* pfParamArr[10], bool& writeAnswer);

        inline void SetLightControllerExecutor(Executor* p){ m_pLightCtrlExecutor = p;}
        inline void SetFanControllerExecutor(Executor* p){ m_pFanCtrlExecutor = p;}
        inline void SetRelayControllerExecutor(Executor* p){ m_pRelayCtrlExecutor = p;}


        inline bool CommandDataAvailable(IO_Device currentIODevice) { return (currentIODevice == SERIAL_PORT0) ? (Serial.available() && Serial.peek() != 0) : (Serial1.available() && Serial1.peek() != 0);}
        inline int ReadCommandData(IO_Device currentIODevice) { return (currentIODevice == SERIAL_PORT0) ? Serial.read() : Serial1.read();}

        void RequestRemoteConnection();

    protected:
        void WriteString(IO_Device dev, const char* szVal);
        void WriteLong(IO_Device dev, long lVal);
        void WriteInt(IO_Device dev, int iVal);
        void WriteFloat(IO_Device dev, float fVal);

    private:
        void CheckCommandAvailabe(IO_Device device);

        IO_Device m_iRequestingDevice;

        Executor* m_pLightCtrlExecutor;
        Executor* m_pFanCtrlExecutor;
        Executor* m_pRelayCtrlExecutor;

        byte commandPos;

        unsigned long m_lTestMillis;
        unsigned long m_lWriteWaitMillis;

        long m_lSessionID;
        bool m_bRemoteControllerActive;
        bool m_bConnectRequested;
        Mode m_CurrentMode;

        char m_CommandLine[MAX_CMD_LEN];
};

#endif // COMMUNICATOR_H
