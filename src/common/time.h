// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_TIME_H
#define USRV_TIME_H

#include <time.h>

namespace usrv
{
    clock_t Clock();
    int NextMinuteInterval(int second = 0);
    int NextHourInterval(int minute = 0, int second = 0);
    int NextDayInterval(int hour = 0, int minute = 0, int second = 0);
    int NextWeekInterval(int wday = 0, int hour = 0, int minute = 0, int second = 0);
    int NextMonthInterval(int mday = 0, int hour = 0, int minute = 0, int second = 0);
}

#endif // USRV_TIME_H
