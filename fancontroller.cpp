#include <Arduino.h>
#include <stdarg.h>
#include "onewire.h"
#include "defines.h"
#include "onewiretempsensor.h"
#include "tempsense.h"
#include "fancontroller.h"
#include "csensor.h"

#define INTERVAL_TEMP 5000UL

#define SPEED 0
#define MAX_TEMP 1

CFanController::CFanController(int id, OneWire* pDS, byte* pSensorAdress, byte* pSensorAdress2, byte* pSensorAdress3, byte* p20WSensor) :CSensor(INTERVAL_TEMP)
{
    m_iCurrentSpeed = 0;
    m_fLastMaxTemp = 0;

    m_pTempSensorDS = new COneWireTempSensor(id, pDS, pSensorAdress, INTERVAL_TEMP);
    m_pTempSensorDS2 = new COneWireTempSensor(id + 1, pDS, pSensorAdress2, INTERVAL_TEMP);
    m_pTempSensorDS3 = new COneWireTempSensor(id + 2, pDS, pSensorAdress3, INTERVAL_TEMP);

    m_pSensor20Watt = new COneWireTempSensor(id + 3, pDS, p20WSensor, INTERVAL_TEMP);

    m_iLastPercent = -1;

}

CFanController::~CFanController()
{
    //dtor
}

void CFanController::Setup()
{
    pinMode(2, OUTPUT);

	TCCR3A = 0;                 // clear control register A
	ICR3 = 320;
	TCCR3B = _BV(WGM33) | _BV(CS30); // set mode as phase and frequency correct pwm, stop the timer
	pinMode(2, OUTPUT);
	TCCR3A |= _BV(COM3B1);

// Timer 3 25khz setting
/*
    TCCR3A = 0;
    TCCR3B = 0;
    TCNT3 = 0;
    OCR3A = 79;   // toggle after counting to 8
    TCCR3A |= (1 << COM3B1);   // Toggle OC1A on Compare Match.
    TCCR3B |= (1 << WGM32);    // CTC mode
    TCCR3B |= (1 << CS31);     // clock on, no pre-scaler

    TIMSK3 |= (1 << OCIE3A);
*/

/*
    TCCR3A = _BV(COM3B1) | _BV(WGM31) | _BV(WGM30); // Just enable output on Pin 3 and disable it on Pin 11
    TCCR3B = _BV(WGM32) | _BV(CS32);
    OCR3A = 255; // defines the frequency 51 = 38.4 KHz, 54 = 36.2 KHz, 58 = 34 KHz, 62 = 32 KHz
    OCR3B = 0;  // deines the duty cycle - Half the OCR2A value for 50%
    TCCR3B = TCCR3B & 0b00111000 | 0x1; // select a prescale value of 8:1 of the system clock
    TIMSK3 |= (1 << OCIE3A);
*/
    m_pTempSensorDS->Setup();
    m_pTempSensorDS2->Setup();
    m_pTempSensorDS3->Setup();
     m_pSensor20Watt->Setup();

    m_pTempSensorDS->Update();
    m_pTempSensorDS2->Update();
    m_pTempSensorDS3->Update();

    m_pSensor20Watt->Update();

    SetSpeed(0);

    m_lLastMillis = millis();

    Serial.println("FC: set up.");

    CSensor::Setup();
}

float CFanController::GetSensorValue()
{
    return 0.0f;
}

void CFanController::OnUpdate()
{
    if(millis() - m_lLastMillis > INTERVAL_TEMP)
    {


        float fTemp = 0.0f;
        float fTemp2 = 0.0f;
        float fTemp3 = 0.0f;

        //float fTemp20W = 0.0f;
        //fTemp20W = m_pSensor20Watt->GetSensorValue();
        //Serial.print("FC: 20W temp(0): ");
        //Serial.println(fTemp20W);

        fTemp = m_pTempSensorDS2->GetSensorValue();

        //Serial.print("FC: max temp(0): ");
        //Serial.println(fTemp);
        fTemp2 = m_pTempSensorDS->GetSensorValue();
        //Serial.print("FC: max temp(1): ");
        //Serial.println(fTemp2);
        fTemp3 = m_pTempSensorDS3->GetSensorValue();
        //Serial.print("FC: max temp(2): ");
        //Serial.println(fTemp3);

        if(fTemp2 > fTemp) fTemp = fTemp2;
        if(fTemp3 > fTemp) fTemp = fTemp3;


        if(fTemp != m_fLastMaxTemp)
        {
            m_fLastMaxTemp = fTemp;

            int speed = 0;
            bool changeSpeed = false;

            if(fTemp < 26.0f)
            {
                speed = 0;
                changeSpeed = true;
                // turn off here!!
            }
            if(fTemp > 30.0f && fTemp <= 35.0f)
            {
                speed = 25;
                changeSpeed = true;
            }
            if(fTemp > 35.0f && fTemp <= 40.0f)
            {
                speed = 50;
                changeSpeed = true;
            }
            if(fTemp > 40)
            {
                speed = 100;
                changeSpeed = true;
            }

            if(changeSpeed )
            {
                Serial.print("FC: max temp: ");
                Serial.println(fTemp);
                SetSpeed(speed);
            }

        }
        m_lLastMillis = millis();
    }

    m_pTempSensorDS->Update();
    m_pTempSensorDS2->Update();
    m_pTempSensorDS3->Update();
    m_pSensor20Watt->Update();

}

void CFanController::SetSpeed(int speedInPercent)
{
    if(speedInPercent != m_iLastPercent)
    {
        m_iLastPercent = speedInPercent;
    }
    else
    {
        return;
    }

    m_iCurrentSpeed = speedInPercent;

    if(m_iCurrentSpeed <= 4) m_iCurrentSpeed = 4;

    //Serial.print("FC: speed in percent: ");
    //Serial.println(speedInPercent);

    unsigned long duty = ((float)m_iCurrentSpeed / 100.0f) * 1023.0f;
    unsigned long dutyCycle = 320;

	dutyCycle *= duty;

	dutyCycle >>= 10;

	OCR3B = dutyCycle;
/*
    char buf[16];
    sprintf(buf, "FC: speed: %d", m_iCurrentSpeed);
    Serial.println(buf);
    Serial.print("dc: ");
    Serial.println(dutyCycle);*/
}

void CFanController::SetValue(int value, float fValue)
{
    switch(value)
    {
    case SPEED:
        {
            Serial.print("FC: SetSpeed: ");
            Serial.println(fValue);

            SetSpeed((int)fValue);
        }
        break;
    default:
        {
            Serial.println("FC: unknown value.");
        }
        break;
    }
}

float CFanController::GetValue(int value)
{
    switch(value)
    {
    case SPEED:
        {
            return m_iCurrentSpeed;
        }
        break;
    case MAX_TEMP:
        {
            return m_fLastMaxTemp;
        }
        break;
    default:
        {
            Serial.println("FC: unknown value.");
        }
        break;
    }
}

long CFanController::Execute(int method, int params, char* pfParamArr[10], bool& writeAnswer)
{
    Serial.println("FC: unknown method.");
}
