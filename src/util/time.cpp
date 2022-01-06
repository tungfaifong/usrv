// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "time.h"

NAMESPACE_OPEN

time_t Now()
{
	return time(NULL);
}

intvl_t ClockMs(clock_t clk)
{
	return clk / (CLOCKS_PER_SEC / SEC2MILLISEC);
}

intvl_t ClockMs()
{
	return ClockMs(clock());
}

int TimeZone(bool recal/* = false */)
{
	static int TIME_ZONE = -1;
	if (TIME_ZONE == -1 || recal)
	{
		time_t now = Now();
		tm local_tm;
		tm gm_tm;
		localtime_r(&now, &local_tm);
		gmtime_r(&now, &gm_tm);
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
	localtime_r(&now, &now_tm);

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

NAMESPACE_CLOSE
