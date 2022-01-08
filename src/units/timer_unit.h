// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_TIMER_UNIT_H
#define USRV_TIMER_UNIT_H

#include <functional>
#include <queue>
#include <time.h>

#include "unit.h"
#include "util/time.h"

NAMESPACE_OPEN

class Timer
{
public:
	Timer(sys_clock_t time, std::function<void()> && callback):_time(time), _callback(std::move(callback)) {}
	virtual ~Timer() {}

	bool operator > (const Timer & t) const
	{
		return _time > t._time;
	}

private:
	sys_clock_t _time;
	std::function<void()> _callback;
	friend class TimerUnit;
};

class TimerUnit : public Unit
{
public:
	TimerUnit(size_t key): Unit(key) {}
	virtual ~TimerUnit() = default;

	virtual bool Start() override final;
	virtual void Update(intvl_t interval) override final;
	virtual void Stop() override final;

public:
	void CreateTimer(intvl_t time, std::function<void()> && callback);

private:
	std::priority_queue<Timer, std::vector<Timer>, std::greater<Timer>> _timers;
};

NAMESPACE_CLOSE

#endif // USRV_TIMER_UNIT_H
