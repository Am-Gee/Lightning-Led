#ifndef DAYTIMEWATCH_H
#define DAYTIMEWATCH_H


class Time
{
public:
    byte m_hour;
    byte m_minute;
    byte m_second;



    Time();
    ~Time();

    void addTime(byte hours, byte minutes);

    Time(byte hour, byte minute);

    void set(Time& source);

    void set(byte hour, byte minute);

    void printTime();

};

class DayTimeWatch
{
public:

    enum DayTime
    {
        DT_NIGHT,
        DT_DAY,
        DT_SUNRISE,
        DT_SUNRISE_OT,
        DT_SUNSET,
        DT_SUNSET_OT
    };

    DayTimeWatch();
    ~DayTimeWatch();

    void checkTime(DayTimeWatch& watch, byte hour, byte minute);
    bool isBefore(Time& time1, Time& time2);
    bool isBetween(Time& time, Time& startTime, Time& endTime);
    byte getOverTime(Time& currentTime, Time& time2, bool overMidnight);
    byte getOverTime();
    void setup(Time& sunrise, Time& sunset, byte duration);
    void setSunsetSunriseDuration(byte minutes);

    byte GetSunriseHour()
    {
        return m_sunriseStart.m_hour;
    }

    void setSunriseHour(byte hour)
    {
        m_sunriseStart.m_hour = hour;
    }

    byte GetSunriseMinute()
    {
        return m_sunriseStart.m_minute;
    }

    void setSunriseMinute(byte minute)
    {
        m_sunriseStart.m_minute = minute;
    }

    void setSunrise(byte hour, byte minute);

    void setSunset(byte hour, byte minute);

    byte GetSunsetMinute()
    {
        return m_sunsetStart.m_minute;
    }

    void setSunsetMinute(byte minute)
    {
        m_sunsetStart.m_minute = minute;
    }

    byte GetSunsetHour()
    {
        return m_sunsetStart.m_hour;
    }

    void setSunsetHour(byte hour)
    {
        m_sunsetStart.m_hour = hour;
    }

    void calculateNightTimeStart();
    DayTimeWatch::DayTime watch(Time& currentTime);

private:
    byte m_overTime;

    byte m_duration;


    Time m_sunriseStart;
    Time m_sunriseEnd;

    Time m_sunsetStart;

    Time m_nightTimeStart;
    Time m_nightTimeEnd;


};

#endif // EXECUTOR_H
