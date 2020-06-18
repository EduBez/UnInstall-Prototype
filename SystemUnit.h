#ifndef SYSTEM_UNIT_H
    #define SYSTEM_UNIT_H

#include <cstdio>
#include <ctime>
#include <string>
#include <vector>

using std::localtime;
using std::sprintf;
using std::string;
using std::time;
using std::time_t;
using std::tm;
using std::tzset;
using std::vector;


//
//  DateTime
//
struct DateTime
{
    static const string Now()
    {
        tzset();
        time_t ttime;
        time(&ttime);

        struct tm ltime = *(localtime(&ttime));
        char buff[22] = "";
        int year = ltime.tm_year + 1900;

        sprintf(buff, "%02d/%02d/%04d - %02d:%02d:%02d",
                ltime.tm_mday, ltime.tm_mon + 1, year,
                ltime.tm_hour, ltime.tm_min, ltime.tm_sec);

        return string(buff);
    }
};


//
//  Util
//
struct Util
{
    static vector<string> GetFixedDrives();
    static string GetNetBIOSName();

    static bool IsBlockMode();
    static bool IsPreview();
    //MARCEL - Change
    static bool IsDifference();

    static string LowerCase (const string& value);
    static string UpperCase (const string& value);
    static string ProperCase(const string& value);

    static string Trim(const string& value);
};

#endif

