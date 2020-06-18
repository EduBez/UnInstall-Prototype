#ifndef TDATETIME_H
    #define TDATETIME_H

#include <time.h>
#include <stdio.h>

#include <string>
using std::string;

///////////////////////////////////////////////////////////////////////
//  TTime Declaration
//
class TTime
{
public:
    // Construtores
    TTime();
    TTime(const int Hour, const int Min, const int Sec=0);
    TTime(const TTIME TqlTime);
    TTime(const time_t UnixTime);

    // Operadores
    friend TTime operator +(const int lhs, const TTime& rhs );
    friend TTime operator -(const int lhs, const TTime& rhs );

    TTime operator +(const int rhs)    const;
    TTime operator -(const int rhs)    const;

    bool operator >(const TTime& rhs)  const
        { return m_secs > rhs.m_secs; }

    bool operator <(const TTime& rhs)  const
        { return m_secs < rhs.m_secs; }

    bool operator ==(const TTime& rhs) const
        { return m_secs == rhs.m_secs; }

    bool operator !=(const TTime& rhs) const
        { return m_secs != rhs.m_secs; }

    bool operator >=(const TTime& rhs) const
        { return m_secs >= rhs.m_secs; }

    bool operator <=(const TTime& rhs) const
        { return m_secs <= rhs.m_secs; }

    const string AsStr  () const;
    const string AsTime () const;
    const time_t AsLong () const;

private:
    time_t    m_secs;
    struct tm m_time;
};

///////////////////////////////////////////////////////////////////////
//  TDate Declaration
//
class TDate
{
public:
    // Construtores
    TDate();
    TDate(const int Day, const int Month, const int Year);
    TDate(const TDATE TqlDate);
    TDate(const time_t UnixTime);

    // Operadores
    friend TDate operator +(const int lhs, const TDate& rhs );
    friend TDate operator -(const int lhs, const TDate& rhs );

    TDate operator +(const int rhs)    const;
    TDate operator -(const int rhs)    const;

    bool operator >(const TDate& rhs)  const
        { return m_secs > rhs.m_secs; }

    bool operator <(const TDate& rhs)  const
        { return m_secs < rhs.m_secs; }

    bool operator ==(const TDate& rhs) const
        { return m_secs == rhs.m_secs; }

    bool operator !=(const TDate& rhs) const
        { return m_secs != rhs.m_secs; }

    bool operator >=(const TDate& rhs) const
        { return m_secs >= rhs.m_secs; }

    bool operator <=(const TDate& rhs) const
        { return m_secs <= rhs.m_secs; }

    const string AsStr  () const;
    const string AsDate () const;
    const time_t AsLong () const;

private:
    time_t    m_secs;
    struct tm m_time;
};

#endif

