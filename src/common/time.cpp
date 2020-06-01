// Copyright (c) 2019-2020 TungFai Fong 

#include <time.h>

#ifdef __unix__

struct tm * LocalTime(const time_t * timer, struct tm * buf)
{
    return localtime_r(timer, buf);
}

#elif _WIN32 // __unix__

errno_t __CRTDECL LocalTime(const time_t * timer, struct tm * buf)
{
    return localtime_s(buf, timer);
}

#endif // _WIN32

namespace usrv
{
    static constexpr int MINUTE_SEC = 60;
    static constexpr int HOUR_SEC = 60 * MINUTE_SEC;
    static constexpr int DAY_SEC = 24 * HOUR_SEC;
    static constexpr int WEEK_SEC = 7 * DAY_SEC;

#define CHECK_SECOND(sec)\
    {\
        if (sec < 0 || sec >= 60)\
        {\
            return -1;\
        }\
    }

#define CHECK_MINUTE(min)\
    {\
        if (min < 0 || min >= 60)\
        {\
            return -1;\
        }\
    }

#define CHECK_HOUR(hour)\
    {\
        if (hour < 0 || hour >= 24)\
        {\
            return -1;\
        }\
    }

#define CHECK_WDAY(wday)\
    {\
        if (wday < 0 || wday >= 7)\
        {\
            return -1;\
        }\
    }

#define CHECK_MONTH(month)\
    {\
        if (month < 0 || month >= 12)\
        {\
            return -1; \
        }\
    }

#define CHECK_MDAY(mday, month_day)\
    {\
        if (mday < 0 || mday >= month_day - 1)\
        {\
            return -1;\
        }\
    }

    inline int GetMonthDay(int year, int month)
    {
        CHECK_MONTH(month);

        static constexpr int MONTH_DAY[12] = { 31,-1,31,30,31,30,31,31,30,31,30,31 };
        int month_day = MONTH_DAY[month];
        if (1 == month) {
            month_day = (((0 == year % 4) && (0 != year % 100) || (0 == year % 400)) ? 29 : 28);
        }
        return month_day;
    }

    clock_t Clock()
    {
        return clock() / (CLOCKS_PER_SEC / 1000);
    }

    int NextMinuteInterval(int second = 0)
    {
        CHECK_SECOND(second);

        time_t now = time(NULL);
        tm now_tm;
        LocalTime(&now, &now_tm);
        return MINUTE_SEC - now_tm.tm_sec + second;
    }

    int NextHourInterval(int minute = 0, int second = 0)
    {
        CHECK_SECOND(second);
        CHECK_MINUTE(minute);

        time_t now = time(NULL);
        tm now_tm;
        LocalTime(&now, &now_tm);
        return HOUR_SEC - (now_tm.tm_min * MINUTE_SEC) - now_tm.tm_sec + minute * MINUTE_SEC + second;
    }

    int NextDayInterval(int hour = 0, int minute = 0, int second = 0)
    {
        CHECK_SECOND(second);
        CHECK_MINUTE(minute);
        CHECK_HOUR(hour);

        time_t now = time(NULL);
        tm now_tm;
        LocalTime(&now, &now_tm);
        return DAY_SEC - (now_tm.tm_hour * HOUR_SEC) - (now_tm.tm_min * MINUTE_SEC) - now_tm.tm_sec + hour * HOUR_SEC + minute * MINUTE_SEC + second;
    }

    int NextWeekInterval(int wday = 0, int hour = 0, int minute = 0, int second = 0)
    {
        CHECK_SECOND(second);
        CHECK_MINUTE(minute);
        CHECK_HOUR(hour);
        CHECK_WDAY(wday);

        time_t now = time(NULL);
        tm now_tm;
        LocalTime(&now, &now_tm);

        int now_wday = now_tm.tm_wday == 0 ? 7 : now_tm.tm_wday;
        return WEEK_SEC - ((now_wday - 1) * DAY_SEC) - (now_tm.tm_hour * HOUR_SEC) - (now_tm.tm_min * MINUTE_SEC) - now_tm.tm_sec + wday * DAY_SEC + hour * HOUR_SEC + minute * MINUTE_SEC + second;
    }

    int NextMonthInterval(int mday = 0, int hour = 0, int minute = 0, int second = 0)
    {
        CHECK_SECOND(second);
        CHECK_MINUTE(minute);
        CHECK_HOUR(hour);

        time_t now = time(NULL);
        tm now_tm;
        LocalTime(&now, &now_tm);

        int now_year = 1900 + now_tm.tm_year;
        int now_month = now_tm.tm_mon;
        int now_month_day = GetMonthDay(now_year, now_month);
        int next_year = now_month == 11 ? now_year + 1 : now_year;
        int next_month = now_month == 11 ? 0 : now_month;
        int next_month_day = GetMonthDay(next_year, next_month);

        CHECK_MDAY(mday, next_month_day);

        return now_month_day * DAY_SEC - ((now_tm.tm_mday - 1) * DAY_SEC) - (now_tm.tm_hour * HOUR_SEC) - (now_tm.tm_min * MINUTE_SEC) - now_tm.tm_sec + mday * DAY_SEC + hour * HOUR_SEC + minute * MINUTE_SEC + second;
    }
}
