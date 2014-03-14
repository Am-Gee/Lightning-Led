#include <Arduino.h>
#include "string.h"

#include "cmodule.h"
#include "communicator.h"

#define RUN_METHOD "run"
#define SETVALUE "sv"
#define GETVALUE "gv"

#define METHOD_SAY_HELLO 10
#define METHOD_START_SESSION 11

#define REMOTE_CONTROLLER_PING_INTERVAL 5000UL
#define MODE_SWITCH_WAIT_TIME 10UL

Communicator::Communicator()
{
    m_bConnectRequested = false;
    m_bSleepRequested = true;
    m_bRemoteControllerActive = false;
    m_iRequestingDevice = SERIAL_PORT0;
    m_CurrentMode = READ;
    commandPos = 0;
    m_CommandLine[0] = '\0';
}

Communicator::~Communicator()
{
    //dtor
}

void Communicator::Setup()
{
    Serial1.begin(9600);

    m_lTestMillis = millis();

    m_lSessionID = millis();

    while(Serial1.available())
    {
        Serial1.read();
    }
    Serial.print("CC: set up.");
}

void Communicator::CheckCommandAvailabe(IO_Device device)
{
    if(CommandDataAvailable(device))
    {
        if(commandPos < MAX_CMD_LEN - 1)
        {
            m_CommandLine[commandPos] = ReadCommandData(device);
            if(m_CommandLine[commandPos++] == '\n')
            {
                if(commandPos < MAX_CMD_LEN - 1) m_CommandLine[commandPos] = '\0';

                m_CurrentMode = EXECUTE;

                m_iRequestingDevice = device;
            }

        }
        else if(m_CommandLine[MAX_CMD_LEN - 1] != '\n')
        {
            Serial.print("COM: CNTOTL:"); // common not terminated or to long
            Serial.println(m_CommandLine);
            memset(m_CommandLine, 0, sizeof(m_CommandLine));
            commandPos = 0;
        }

    }
}

void Communicator::WriteFloat(IO_Device dev, float fVal)
{
    m_CurrentMode = WRITE;
    if(dev == SERIAL_PORT0)
    {
        Serial.println(fVal);
    }
    else
    {
       Serial1.println(fVal);
    }
    m_lWriteWaitMillis = millis();
}

void Communicator::WriteInt(IO_Device dev, int iVal)
{
    m_CurrentMode = WRITE;
    if(dev == SERIAL_PORT0)
    {
        Serial.println(iVal);
    }
    else
    {
       Serial1.println(iVal);
    }
    m_lWriteWaitMillis = millis();
}

void Communicator::WriteLong(IO_Device dev, long lVal)
{
    m_CurrentMode = WRITE;

    if(dev == SERIAL_PORT0)
    {
        Serial.println(lVal);
    }
    else
    {
       Serial1.println(lVal);
    }
    m_lWriteWaitMillis = millis();
}


void Communicator::WriteString(IO_Device dev, const char* szVal)
{
    m_CurrentMode = WRITE;
    if(dev == SERIAL_PORT0)
    {
        Serial.println(szVal);
    }
    else
    {
       Serial1.println(szVal);
    }
    m_lWriteWaitMillis = millis();
}

void Communicator::RequestRemoteConnection()
{
    m_bConnectRequested = false;
    m_lTestMillis = millis();
}

void Communicator::RequestSleepMode()
{
    m_bSleepRequested = false;
    m_lTestMillis = millis();
}


void Communicator::Update()
{
    if(m_CurrentMode == WRITE)
    {
        if(millis() - m_lWriteWaitMillis > MODE_SWITCH_WAIT_TIME)
        {
            m_lWriteWaitMillis = 0;
            m_CurrentMode = READ;
        }
        else
        {
            return;
        }
    }
    if(m_CurrentMode == READ)
    {
        if(!m_bConnectRequested && millis() - m_lTestMillis > REMOTE_CONTROLLER_PING_INTERVAL)
        {
            m_lTestMillis = millis();

            WriteString(SERIAL_PORT1, "CC:CONNECT");

            m_bConnectRequested = true;

            return;
        }

        if(!m_bSleepRequested && millis() - m_lTestMillis > REMOTE_CONTROLLER_PING_INTERVAL)
        {
            m_lTestMillis = millis();

            WriteString(SERIAL_PORT1, "CC:SLEEP");

            m_bSleepRequested = true;

            return;
        }

        CheckCommandAvailabe(SERIAL_PORT0);

        if(m_CurrentMode == READ)
            CheckCommandAvailabe(SERIAL_PORT1);
    }
    else if(m_CurrentMode == EXECUTE)
    {
        m_lWriteWaitMillis = 0;

        long start = micros();

        char *str;
        char *p;

        byte cmdCount = 0;

        char* command = NULL;
        char* target = NULL;
        char* paramStart = NULL;

        char* paramArr[10];

        str = strtok_r(m_CommandLine, " ", &p);
        target = p;

        byte argCount = 0;

        while(str != NULL)
        {
            if(cmdCount == 0) command = str;
            if(cmdCount == 1) target = str;

            if(cmdCount >= 2 && cmdCount < 10)
            {
                paramArr[argCount] = str;
                argCount++;
            }
            str = strtok_r(NULL, " ", &p);
            cmdCount++;
        }

        Executor* pTargetExecutor = NULL;
        if(strcmp(target, "lc") == 0)
        {
            pTargetExecutor = m_pLightCtrlExecutor;
        }
        else if(strcmp(target, "fc") == 0)
        {
            pTargetExecutor = m_pFanCtrlExecutor;
        }
        else if(strcmp(target, "rc") == 0)
        {
            pTargetExecutor = m_pRelayCtrlExecutor;
        }
        else if(strcmp(target, "cc") == 0)
        {
            pTargetExecutor = this;
        }

        if(pTargetExecutor != NULL)
        {
            if(strcmp(command, SETVALUE) == 0) // SetMode
            {
                if(argCount == 0)
                    Serial.println("CC: sv missing argument.");

                pTargetExecutor->SetValue(atoi(paramArr[0]), atof(paramArr[1]));
                WriteInt(m_iRequestingDevice, 0);
            }
            else if(strcmp(command, GETVALUE) == 0)
            {
                float fVal = pTargetExecutor->GetValue(atoi(paramArr[0]));

                WriteFloat(m_iRequestingDevice, fVal);

            }
            else if(strcmp(command, RUN_METHOD) == 0)
            {
                char* pParams[argCount - 1];
                for(int x = 0; x < argCount - 1; x++)
                {
                    pParams[x] = paramArr[x + 1];
                }
                bool writeAnswer = true;

                long result = pTargetExecutor->Execute(atoi(paramArr[0]), argCount - 1, pParams, writeAnswer);

                if(writeAnswer)
                    WriteLong(m_iRequestingDevice, result);
            }
        }
        else
        {
            Serial.println("CC: target unknown.");
            m_iRequestingDevice = SERIAL_PORT0;
            m_CurrentMode = READ;
        }

        memset(m_CommandLine, 0, sizeof(m_CommandLine));

        commandPos = 0;

/*
        long time = micros() - start;
        Serial.print("parse: " );
        Serial.println(time);*/
    }


}

void Communicator::SetValue(int value, float fValue)
{

}

float Communicator::GetValue(int value)
{

}

long Communicator::Execute(int method, int params, char* pfParamArr[10], bool& writeAnswer)
{
    switch(method)
    {
    case METHOD_SAY_HELLO:
        {
            Serial.print("CC: Hello World: ");
            for(int x = 0; x < params; x++)
            {
                Serial.print(pfParamArr[x]);
                Serial.print(" ");
            }
            Serial.println();
            if(m_iRequestingDevice == SERIAL_PORT1)
            {
                WriteString(m_iRequestingDevice, "CC: Hello World!!");
                writeAnswer = false;
            }

        }
        break;
    case METHOD_START_SESSION:
        {
            if(params != 0) Serial.println("CC: illegal params.");

            m_bRemoteControllerActive = true;

            Serial.println("CC: remote session active.");

            return m_lSessionID;
        }
        break;
    default:
        {
            Serial.println("CC: unknown method.");
        }
        break;
    }
    return 0;
}
