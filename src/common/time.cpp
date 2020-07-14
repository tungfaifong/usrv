// Copyright (c) 2019-2020 TungFai Fong 

#include "time.h"

namespace usrv
{
    inline bool CheckSecond(int sec)
    {
        return sec >= 0 && sec < 60;
    }

    inline bool CheckMinute(int min)
    {
        return min >= 0 && min < 60;
    }

    inline bool CheckHour(int hour)
    {
        return hour >= 0 && hour < 24;
    }

    inline bool CheckWDay(int wday)
    {
        return wday >= 0 && wday < 7;
    }

    inline bool CheckMonth(int mon)
    {
        return mon >= 0 && mon < 12;
    }

    inline int GetMaxMDay(int year, int month)
    {
        static constexpr int MAX_MDAY[12] = { 31,-1,31,30,31,30,31,31,30,31,30,31 };
        int max_mday = MAX_MDAY[month];
        if (1 == month) {
            max_mday = (((0 == year % 4) && (0 != year % 100) || (0 == year % 400)) ? 29 : 28);
        }
        return max_mday;
    }

    inline bool CheckMDay(int mday, int year, int month)
    {
        return mday > 0 && mday <= GetMaxMDay(year, month);
    }

    clock_t Clock()
    {
        return clock() / (CLOCKS_PER_SEC / 1000);
    }

    time_t Now()
    {
        return time(NULL);
    }

    int TimeZone(bool recal)
    {
        static int TIME_ZONE = -1;
        if (TIME_ZONE == -1 || recal)
        {
            time_t now = Now();
            tm local_tm;
            tm gm_tm;
            LocalTime(&now, &local_tm);
            GMTime(&now, &gm_tm);
            TIME_ZONE = local_tm.tm_hour - gm_tm.tm_hour;
        }
        return TIME_ZONE;
    }

    int NextMinuteInterval(int second)
    {
        if (!CheckSecond(second))
        {
            return -1;
        }

        time_t now = Now();
        return MINUTE_SEC - now % MINUTE_SEC + second;
    }

    int NextHourInterval(int minute, int second)
    {
        if (!CheckMinute(minute) || !CheckSecond(second))
        {
            return -1;
        }

        time_t now = Now();
        return HOUR_SEC - now % HOUR_SEC + minute * MINUTE_SEC + second;
    }

    int NextDayInterval(int hour, int minute, int second)
    {
        if (!CheckHour(hour) || !CheckMinute(minute) || !CheckSecond(second))
        {
            return -1;
        }

        time_t now = Now();
        return DAY_SEC - TimeZone() * HOUR_SEC - now % DAY_SEC + hour * HOUR_SEC + minute * MINUTE_SEC + second;
    }

    int NextWeekInterval(int wday, int hour, int minute, int second)
    {
        if (!CheckWDay(wday) || !CheckHour(hour) || !CheckMinute(minute) || !CheckSecond(second))
        {
            return -1;
        }

        time_t now = Now();
        return WEEK_SEC - TimeZone() * HOUR_SEC - (now - 4 * DAY_SEC) % WEEK_SEC + wday * DAY_SEC + hour * HOUR_SEC + minute * MINUTE_SEC + second;
    }

    int NextMonthInterval(int mday, int hour, int minute, int second)
    {
        if (!CheckHour(hour) || !CheckMinute(minute) || !CheckSecond(second))
        {
            return -1;
        }

        time_t now = Now();
        tm now_tm;
        LocalTime(&now, &now_tm);

        int now_year = 1900 + now_tm.tm_year;
        int now_month = now_tm.tm_mon;
        int now_max_mday = GetMaxMDay(now_year, now_month);
        int next_year = now_month == 11 ? now_year + 1 : now_year;
        int next_month = ++now_month % 12;

        if (!CheckMDay(mday, next_year, next_month))
        {
            return -1;
        }

        return now_max_mday * DAY_SEC - ((now_tm.tm_mday - 1) * DAY_SEC) - (now_tm.tm_hour * HOUR_SEC) - (now_tm.tm_min * MINUTE_SEC) - now_tm.tm_sec + (mday - 1) * DAY_SEC + hour * HOUR_SEC + minute * MINUTE_SEC + second;
    }
}
