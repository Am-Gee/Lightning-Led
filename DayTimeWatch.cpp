#include <Arduino.h>
#include "daytimewatch.h"

void Time::printTime()
{
    char buf[10];
    sprintf(buf, "%02d:%02d:%02d", m_hour, m_minute, m_second);
    Serial.println(buf);
}

void Time::addTime(byte hours, byte minutes)
{
    byte hoursToAdd = hours;
    byte minutesToAdd = minutes;

    if(minutes > 60)
    {
        byte hoursOver= minutes / 60;

        minutesToAdd = minutes % 60;

        hoursToAdd += hoursOver;
    }

    m_minute += minutesToAdd;
    if(m_minute >= 60)
    {
        m_minute = m_minute - 60;
        hoursToAdd++;
    }
    m_hour += hoursToAdd;
    if(m_hour >= 24)
    {
        m_hour = m_hour - 24;
    }
}

Time::Time()
{
    m_hour = 0;
    m_minute = 0;
    m_second = 0;
}

Time::Time(byte hour, byte minute)
{
    m_hour = hour;
    m_minute = minute;
    m_second = 0;

}

Time::~Time()
{


}

void Time::set(Time& source)
{

    m_hour = source.m_hour;
    m_minute = source.m_minute;
    m_second = source.m_second;
}

void Time::set(byte hour, byte minute)
{
    m_hour = hour;
    m_minute = minute;
}

DayTimeWatch::DayTimeWatch()
{
    m_duration = 0;
}

DayTimeWatch::~DayTimeWatch()
{

}

bool DayTimeWatch::isBetween(Time& time, Time& startTime, Time& endTime)
{
    if(time.m_hour >= startTime.m_hour && time.m_hour <= endTime.m_hour)
    {
        if(startTime.m_hour == endTime.m_hour)
        {
            if(time.m_minute >= startTime.m_minute && time.m_minute < endTime.m_minute)
            {
                return true;
            }
        }
        else if(time.m_hour == startTime.m_hour)
        {
            if(time.m_minute >= startTime.m_minute)
            {
                return true;
            }
        }
        else if(time.m_hour == endTime.m_hour)
        {
            if(time.m_minute < endTime.m_minute)
            {
                return true;
            }
        }
        else
        {
            return true;
        }
    }
    return false;
}

byte DayTimeWatch::getOverTime(Time& currentTime, Time& time2, bool overMidnight)
{
    if(currentTime.m_hour == time2.m_hour && currentTime.m_minute == time2.m_minute) return 0;

    if(!overMidnight)
    {
        byte minutesOver = 0;
        if(isBefore(time2, currentTime))
        {
            minutesOver =  currentTime.m_minute - time2.m_minute;
            minutesOver = minutesOver + ( (currentTime.m_hour - time2.m_hour) * 60);
        }
        return minutesOver;
    }


    // it goes over midnight
    if(isBefore(currentTime, time2)) // it goes over midnight if true 0:30 current, 23:30 time2
    {
        byte minutesAfterMidnight = currentTime.m_hour * 60;
        minutesAfterMidnight += currentTime.m_minute;

        byte hours2MidNight = 0;
        if(time2.m_hour < 23)
        {
            hours2MidNight = 23 - time2.m_hour;
        }

        byte minutesToNextHour = 60 - time2.m_minute;

        byte minutes2MidNight = minutesToNextHour + (60 * hours2MidNight);
        return minutesAfterMidnight + minutes2MidNight;

    }

    byte hours = currentTime.m_hour - time2.m_hour;
    return (hours * 60) + currentTime.m_minute - time2.m_minute;

}

void DayTimeWatch::setup(Time& sunrise, Time& sunset, byte duration)
{
    Serial.print("SUNRISE: " );
    sunrise.printTime();

    Serial.println();

    Serial.print("SUNSET: ");
    sunset.printTime();


    Serial.print("DURATION: ");
    Serial.println(duration);

    m_duration = duration;

    if(m_duration >= 60)
    {
        Serial.println("ERROR: duration too long. using 59 min.");

        m_duration = 59;
    }

    if(sunset.m_hour == sunrise.m_hour)
    {
        if(sunset.m_minute < sunrise.m_minute)
        {
            if(sunrise.m_minute - sunset.m_minute < m_duration)
            {
                Serial.println("ERROR: sunset/sunrise too close together.");
            }
        }
    }

    setSunrise(sunrise.m_hour, sunrise.m_minute);

    setSunset(sunset.m_hour, sunset.m_minute);

    calculateNightTimeStart();
}

void DayTimeWatch::setSunsetSunriseDuration(byte minutes)
{
    m_duration = minutes;
    if(m_duration >= 60)
    {
        Serial.println("ERROR: duration too long. using 59 min.");
        m_duration = 59;
    }

    m_sunriseEnd.set(m_sunriseStart);
    m_sunriseEnd.addTime(0, minutes);

    calculateNightTimeStart();
}

void DayTimeWatch::setSunrise(byte hour, byte minute)
{
    m_sunriseStart.m_hour = hour;
    m_sunriseStart.m_minute = minute;
    m_sunriseEnd.set(m_sunriseStart);

    m_sunriseEnd.addTime(0, m_duration);
}

void DayTimeWatch::setSunset(byte hour, byte minute)
{
    m_sunsetStart.m_hour = hour;
    m_sunsetStart.m_minute = minute;


}

void DayTimeWatch::calculateNightTimeStart()
{

    m_nightTimeStart.m_hour = m_sunsetStart.m_hour;
    m_nightTimeStart.m_minute = m_sunsetStart.m_minute;

    m_nightTimeStart.addTime(0, m_duration + 1);

    m_nightTimeEnd.m_hour = m_sunriseStart.m_hour;
    m_nightTimeEnd.m_minute = m_sunriseStart.m_minute;

    /*if(m_nightTimeStart.hour == m_sunriseStart.hour)
    {
        if(m_nightTimeStart.minute < m_sunriseStart.minute)
        {
            this->nightTimeAfterMidnight = true;
        }
    }
    else if(this->nightTimeStart.hour < this->sunriseStart.hour)
    {
        this->nightTimeAfterMidnight = true;
    }
    */

}



bool DayTimeWatch::isBefore(Time& time1, Time& time2)
{
    if(time1.m_hour <= time2.m_hour)
    {
        if(time1.m_hour == time2.m_hour)
        {
            if(time1.m_minute > time2.m_minute)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

DayTimeWatch::DayTime DayTimeWatch::watch(Time& currentTime)
{
    m_overTime = 0;

    DayTime dayTime = DT_NIGHT;

    if(isBefore(m_nightTimeStart, m_sunriseStart))
    {
        if(!isBetween(currentTime, m_nightTimeStart, m_sunriseStart))
        {
            dayTime = DT_DAY;
        }
    }
    else
    {
        if(isBetween(currentTime, m_sunriseStart, m_nightTimeStart))
        {
            dayTime = DT_DAY;
        }
    }

    // sunset or sunrise??
    if(dayTime == DT_DAY)
    {
        if(isBefore(m_sunriseEnd, m_sunriseStart)) // sunrise over midnight
        {
            if(!isBetween(currentTime, m_sunriseEnd, m_sunriseStart))
            {
                dayTime = DT_SUNRISE;
                m_overTime = getOverTime(currentTime, m_sunriseStart, true);
                if( m_overTime > 0)
                {
                    dayTime = DT_SUNRISE_OT;

                }
            }
        }
        else
        {
            if(isBetween(currentTime, m_sunriseStart, m_sunriseEnd))
            {
                dayTime = DT_SUNRISE;
                m_overTime = getOverTime(currentTime, m_sunriseStart, false);
                if( m_overTime > 0)
                {
                    dayTime = DT_SUNRISE_OT;
                }
            }
        }
    }

    if(dayTime == DT_DAY)
    {
        if(isBefore(m_nightTimeStart, m_sunsetStart)) // sunset over midnight
        {
            if(!isBetween(currentTime, m_nightTimeStart, m_sunsetStart))
            {
                dayTime = DT_SUNSET;
                m_overTime = getOverTime(currentTime, m_sunsetStart, true);
                if( m_overTime > 0)
                {
                    dayTime = DT_SUNSET_OT;
                }
            }
        }
        else
        {
            if(isBetween(currentTime, m_sunsetStart, m_nightTimeStart))
            {
                dayTime = DT_SUNSET;
                m_overTime = getOverTime(currentTime, m_sunsetStart, false);
                if( m_overTime > 0)
                {
                    dayTime = DT_SUNSET_OT;
                }
            }
        }
    }
    return dayTime;
}

byte DayTimeWatch::getOverTime()
{
    return m_overTime;
}
