// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_TIMER_UNIT_H
#define USRV_TIMER_UNIT_H

#include <functional>
#include <memory>
#include <queue>
#include <time.h>

#include "unit.h"

namespace usrv
{

class Timer
{
public:
	Timer(intvl_t time, std::function<void()> && callback):_time(time), _callback(std::move(callback)) {}
	virtual ~Timer() {}

	bool operator > (const Timer & t) const
	{
		return _time > t._time;
	}

private:
	intvl_t _time;
	std::function<void()> _callback;
	friend class TimerUnit;
};

class TimerUnit : public Unit
{
public:
	TimerUnit();
	virtual ~TimerUnit();

	virtual bool Start() override final;
	virtual void Update(intvl_t interval) override final;
	virtual void Stop() override final;

public:
	void CreateTimer(intvl_t time, std::function<void()> && callback);

private:
	std::priority_queue<Timer, std::vector<Timer>, std::greater<Timer> > _timers;
};

}

#endif // USRV_TIMER_UNIT_H