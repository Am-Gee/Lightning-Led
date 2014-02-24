#include <arduino.h>
#include "relaycontroller.h"

#define MAX_RELAY_PINS 8

#define TOGGLE_RELAY 9

const static byte relayOutputPins[MAX_RELAY_PINS]  = {33, 34, 35, 36, 37, 38, 39, 40};
static byte pinState[MAX_RELAY_PINS] = {1, 1, 1, 1, 1, 1,1, 1};

RelayController::RelayController()
{
    m_bPinStateChanged = false;
}

RelayController::~RelayController()
{
    //dtor
}

void RelayController::Setup()
{
    CModule::Setup();

    for(int i = 0; i < MAX_RELAY_PINS; i++)
    {
        pinMode(relayOutputPins[i], OUTPUT);
        digitalWrite(relayOutputPins[i], 1);
    }
    Serial.println("RC: set up.");
}

void RelayController::Update()
{
    if(m_bPinStateChanged)
    {
        for(int i = 0; i < MAX_RELAY_PINS; i++)
        {
            digitalWrite(relayOutputPins[i], pinState[i]);
        }
        m_bPinStateChanged = false;
    }
}

float RelayController::GetValue(int value)
{
    if(value >= 0 && value <= 7)
    {
        return digitalRead(relayOutputPins[value]);
    }
    return -1.0f;
}

void RelayController::SetValue(int value, float fValue)
{
    if(value >= 0 && value <= 7)
    {
        SetRelayState(value, (fValue) ? true : false);
    }
}

void RelayController::SetRelayState(int relayNum, bool turnOn)
{
    char buf[32];
    sprintf(buf, "RC: relay(%d): %d", relayNum, turnOn);
    Serial.println(buf);

    if(relayNum >= 0 && relayNum < MAX_RELAY_PINS)
    {
        pinState[relayNum] = !turnOn;
    }
    m_bPinStateChanged = true;
}

long RelayController::Execute(int method, int params, char* pfParamArr[10], bool& writeAnswer)
{
    Serial.println("FC: unknown method.");
}
