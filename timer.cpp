#include <arduino.h>
#include <stdarg.h>
#include <Wire.h>

#include "timer.h"

const int DS1307 = 0x68; // Address of DS1307 see data sheets
const char* days[] =
{"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char* months[] =
{"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug","Sep", "Oct", "Nov", "Dec"};


#define TIMER_UPDATE_INTERVAL  20000UL



CTimer::CTimer()
{
    m_lPreviousMillis = 0L;
    m_lPreviousSecondUpdateMillis = 0L;

    // Initializes all values:
    m_nSecond = 0;
    m_nMinute = 0;
    m_nHour = 0;
    m_nWeekday = 1;
    m_nMonthday = 1;
    m_nMonth = 1;
    m_nYear = 13;
}

CTimer::~CTimer()
{
    //dtor
}

void CTimer::Setup()
{
  CModule::Setup();

  //Serial.begin(9600); // already done in main class

  delay(500); // This delay allows the MCU to read the current date and time.

  Serial.print("TIMER: ");

  ReadTime();
  //SetTime();
  PrintTime();
/*
  Serial.println("Please change to newline ending the settings on the lower right of the Serial Monitor");
  Serial.println("Would you like to set the date and time now? Y/N");

  while (!Serial.available()) delay(10);
  if (Serial.read() == 'y' || Serial.read() == 'Y')

  // This set of functions allows the user to change the date and time
  {
    Serial.read(); // get \n
    SetTime();
    Serial.print("The current date and time is now: ");
    PrintTime();
  }

  Serial.println("Thank you.");
*/

  m_lPreviousMillis = millis();
  m_lPreviousSecondUpdateMillis = millis();
  Serial.println("Timer: set up.");

}

void CTimer::Update()
{
    unsigned long currentMillis = millis();
    bool timeRead = false;

    /*if(currentMillis - m_lPreviousSecondUpdateMillis >= 1000UL)
    {
        m_lPreviousSecondUpdateMillis = currentMillis;

        m_nSecond++;
        if(m_nSecond >= 60)
        {
            ReadTime();
            timeRead = true;
        }
        //PrintTime();
    }*/

    if((currentMillis - m_lPreviousMillis) > TIMER_UPDATE_INTERVAL)
    {
        m_lPreviousMillis = currentMillis;
        ReadTime();
        PrintTime();
    }
}

byte CTimer::decToBcd(byte val)
{
  return ((val/10*16) + (val%10));
}

byte CTimer::bcdToDec(byte val)
{
  return ((val/16*10) + (val%16));
}


// This set of codes is allows input of data
void CTimer::SetTime()
{
    /*
    m_nYear = 14;
    m_nMonth = 2;
    m_nMonthday = 21;
    m_nWeekday = 6;
    m_nHour = 23;
    m_nMinute = 59;
    m_nSecond = 0;
*/

  Serial.print("year, 00-99. - ");
  m_nYear = ReadByte();
  Serial.println(m_nYear);
  Serial.print("month, 1-12. - ");
  m_nMonth = ReadByte();
  Serial.println(months[m_nMonth-1]);
  Serial.print("day, 1-31. - ");
  m_nMonthday = ReadByte();
  Serial.println(m_nMonthday);
  Serial.println("day of the week, 1-7.");
  Serial.print("1 Sun | 2 Mon | 3 Tues | 4 Weds | 5 Thu | 6 Fri | 7 Sat - ");
  m_nWeekday = ReadByte();
  Serial.println(days[m_nWeekday-1]);
  Serial.print("hour in 24hr format, 0-23. - ");
  m_nHour = ReadByte();
  Serial.println(m_nHour);
  Serial.print("minute, 0-59. - ");
  m_nMinute = ReadByte();
  Serial.println(m_nMinute);
  m_nSecond = 0;


  // The following codes transmits the data to the RTC
  Wire.beginTransmission(DS1307);
  Wire.write(byte(0));
  Wire.write(decToBcd(m_nSecond));
  Wire.write(decToBcd(m_nMinute));
  Wire.write(decToBcd(m_nHour));
  Wire.write(decToBcd(m_nWeekday));
  Wire.write(decToBcd(m_nMonthday));
  Wire.write(decToBcd(m_nMonth));
  Wire.write(decToBcd(m_nYear));
  Wire.write(byte(0));
  Wire.endTransmission();
  // Ends transmission of data
}


byte CTimer::ReadByte()
{
  while (!Serial.available()) delay(100);
  byte reading = 0;
  byte incomingByte = Serial.read();
  while (incomingByte != '\n')
  {
    if (incomingByte >= '0' && incomingByte <= '9')
      reading = reading * 10 + (incomingByte - '0');
    else;
    incomingByte = Serial.read();
  }
  Serial.flush();
  return reading;
}

short CTimer::GetHour()
{
    return m_nHour;
}

short CTimer::GetMinute()
{
    return m_nMinute;
}

short CTimer::GetSecond()
{
    return m_nSecond;
}

short CTimer::GetDayOfMonth()
{
    return m_nMonthday;
}

void CTimer::PrintTime()
{
  char buffer[3];
  const char* AMPM = 0;

  Serial.print(days[m_nWeekday-1]);
  Serial.print(" ");
  Serial.print(months[m_nMonth-1]);
  Serial.print(" ");
  Serial.print(m_nMonthday);
  Serial.print(", 20");
  Serial.print(m_nYear);
  Serial.print(" ");

  Serial.print(m_nHour);
  Serial.print(":");
  sprintf(buffer, "%02d", m_nMinute);
  Serial.print(buffer);
  Serial.print(":");
  sprintf(buffer, "%02d", m_nSecond);
  Serial.println(buffer);
}


void CTimer::ReadTime()
{
  Wire.beginTransmission(DS1307);
  Wire.write(byte(0));
  Wire.endTransmission();
  Wire.requestFrom(DS1307, 7);
  m_nSecond = bcdToDec(Wire.read());
  m_nMinute = bcdToDec(Wire.read());
  m_nHour = bcdToDec(Wire.read());
  m_nWeekday = bcdToDec(Wire.read());
  m_nMonthday = bcdToDec(Wire.read());
  m_nMonth = bcdToDec(Wire.read());
  m_nYear = bcdToDec(Wire.read());
}
