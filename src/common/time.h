// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_TIME_H
#define USRV_TIME_H

#include <time.h>

namespace usrv
{
#ifdef __unix__ // __unix__

    struct tm * LocalTime(const time_t * timer, struct tm * buf);
    struct tm * GMTime(const time_t * timer, struct tm * buf);

#elif _WIN32 // _WIN32

    errno_t __CRTDECL LocalTime(const time_t * timer, struct tm * buf);
    errno_t __CRTDECL GMTime(const time_t * timer, struct tm * buf);

#endif 

    static constexpr int MINUTE_SEC = 60;
    static constexpr int HOUR_SEC = 60 * MINUTE_SEC;
    static constexpr int DAY_SEC = 24 * HOUR_SEC;
    static constexpr int WEEK_SEC = 7 * DAY_SEC;

    clock_t Clock();
    time_t Now();
    int TimeZone(bool recal = false);
    int NextMinuteInterval(int second = 0);
    int NextHourInterval(int minute = 0, int second = 0);
    int NextDayInterval(int hour = 0, int minute = 0, int second = 0);
    int NextWeekInterval(int wday = 0, int hour = 0, int minute = 0, int second = 0);
    int NextMonthInterval(int mday = 1, int hour = 0, int minute = 0, int second = 0);
}

#endif // USRV_TIME_H
