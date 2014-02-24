#include <Arduino.h>
#include <stdarg.h>
#include "cmodule.h"

CModule::CModule()
{
    m_lUpdateInterval = 0L;
    m_lLastMillis = 0;
}

CModule::CModule(unsigned long lUpdateInterval)
{
    m_lUpdateInterval = lUpdateInterval;
    m_lLastMillis = 0;
}

CModule::~CModule()
{
    //dtor
}

void CModule::Setup()
{
    m_lLastMillis = millis();
}

void CModule::Update()
{
    unsigned long curMillis = millis();

    if(curMillis - m_lLastMillis >= m_lUpdateInterval || m_lUpdateInterval == 0)
    {
        OnUpdate();

        m_lLastMillis = curMillis;
    }

}

void CModule::OnUpdate()
{
}
