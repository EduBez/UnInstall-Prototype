#include "DateTime.h"

///////////////////////////////////////////////////////////////////////
//  TTime Implementation
//
TTime::TTime()
{
    tzset();
    time(&m_secs);
    m_time = *(localtime(&m_secs));

    struct tm time;
    memset(&time, 0, sizeof(time));
}

TTime::TTime(const int Hour, const int Min, const int Sec)
{
    tzset();
    struct tm time = {Sec, Min, Hour, 1,0,102,0,0,0};

    m_time = time;
    m_secs = mktime(&m_time);
}

TTime::TTime(const TTIME TqlTime)
{
    tzset();
    struct tm time =
        {TqlTime.sec, TqlTime.min, TqlTime.hour, 1,0,102,0,0,0};

    m_time = time;
    m_secs = mktime(&m_time);
}

TTime::TTime(const time_t UnixTime)
{
    tzset();
    struct tm time = {0,0,0,1,0,102,0,0,0};

    m_time = *(localtime(&UnixTime));

    time.tm_sec  = m_time.tm_sec ;
    time.tm_min  = m_time.tm_min ;
    time.tm_hour = m_time.tm_hour;

    m_secs = mktime(&time);
}

TTime TTime::operator +(const int rhs) const
{                          // value in seconds
    time_t time = m_secs + rhs;
    return TTime(time);
}

TTime operator +(const int lhs, const TTime& rhs )
{
    time_t time = rhs.m_secs + lhs;
    return TTime(time);
}

TTime TTime::operator -(const int rhs) const
{                         // value in seconds
    time_t time = m_secs - rhs;
    return TTime(time);
}

TTime operator -(const int lhs, const TTime& rhs )
{
    time_t time = rhs.m_secs - lhs;
    return TTime(time);
}

const string TTime::AsStr()  const
{
    struct tm time;

    time = *(localtime(&m_secs));
    char buff[64] = "";

    sprintf(buff, "%02d:%02d:%02d",
            time.tm_hour, time.tm_min, time.tm_sec);

    return string(buff);
}

const string TTime::AsTime() const
{
    char buff[128] = "";
    sprintf(buff, "%ld", m_secs);
    return string(buff);
}

const time_t TTime::AsLong() const
{
    return m_secs;
}

///////////////////////////////////////////////////////////////////////
//  TDate Implementation
//
TDate::TDate()
{
    tzset();
    time(&m_secs);
    m_time = *(localtime(&m_secs));

    struct tm time;
    memset(&time, 0, sizeof(time));

    time.tm_mday = m_time.tm_mday;
    time.tm_mon  = m_time.tm_mon;
    time.tm_year = m_time.tm_year;

    m_time = time;
    m_secs = mktime(&m_time);
}

TDate::TDate(const int Day, const int Month, const int Year)
{
    memset(&m_time, 0, sizeof(m_time));

    m_time.tm_mday = Day;
    m_time.tm_mon  = Month - 1;
    m_time.tm_year = Year  - 1900;

    m_secs = mktime(&m_time);
}

TDate::TDate(const TDATE TqlDate)
{
    int year;

    if (TqlDate.year >= 90 && TqlDate.year <= 99)
        year = (TqlDate.year + 1900) - 1900;
    else
        year = (TqlDate.year + 2000) - 1900;

    memset(&m_time, 0, sizeof(m_time));

    m_time.tm_mday = TqlDate.day;
    m_time.tm_mon  = TqlDate.month - 1;
    m_time.tm_year = year;

    m_secs = mktime(&m_time);
}

TDate::TDate(const time_t UnixTime)
{
    tzset();
    m_secs = UnixTime;
    m_time = *(localtime(&m_secs));
}

TDate TDate::operator +(const int rhs) const
{
    time_t time = m_secs + (rhs * 24 * 3600);
    return TDate(time);
}

TDate operator +(const int lhs, const TDate& rhs)
{
    time_t time = rhs.m_secs + (lhs * 24 * 3600);
    return TDate(time);
}

TDate TDate::operator -(const int rhs) const
{
    time_t time = m_secs - (rhs * 24 * 3600);
    return TDate(time);
}

TDate operator -(const int lhs, const TDate& rhs)
{
    time_t time = rhs.m_secs - (lhs * 24 * 3600);
    return TDate(time);
}

const string TDate::AsStr()  const
{
    int year = (m_time.tm_year + 1900) - 2000;
    if (year < 0) year *= -1;

    char buff[64] = "";
    sprintf(buff, "%02d/%02d/%02d",
            m_time.tm_mday, m_time.tm_mon + 1, year);

    return string(buff);
}

const string TDate::AsDate() const
{
    char buff[128] = "";
    sprintf(buff, "%ld", m_secs);
    return string(buff);
}

const time_t TDate::AsLong() const
{
    return m_secs;
}

