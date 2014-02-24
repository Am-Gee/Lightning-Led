#include <Arduino.h>
#include <stdarg.h>
#include "defines.h"
#include "tempsense.h"

#define TEMP_PIN 0

CTempSensor::CTempSensor(uint8_t nInputPin, long lUpateInterval) : CSensor(lUpateInterval)
{
    m_nInputPin = nInputPin;
    m_fValue = 0.0f;
}

CTempSensor::~CTempSensor()
{
    //dtor
}

float CTempSensor::GetSensorValue()
{
    return m_fValue;
}

void CTempSensor::Setup()
{
    pinMode(m_nInputPin, INPUT);
}

void CTempSensor::OnUpdate()
{
    m_fValue = ( 5.0 * (float)analogRead(m_nInputPin) * 100.0) / 1024.0;
}
