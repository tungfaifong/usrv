// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_TIMER_UNIT_H
#define USRV_TIMER_UNIT_H

#include <functional>
#include <queue>
#include <time.h>

#include "unit.h"
#include "util/object_list.hpp"
#include "util/object_pool.hpp"
#include "util/time.h"

NAMESPACE_OPEN

class Timer
{
public:
	Timer() = default;
	virtual ~Timer() = default;

	bool operator > (const Timer & t) const
	{
		return _time > t._time;
	}

public:
	void Start(TIMERID id, sys_clock_t time, std::function<void()> && callback);
	void Stop();

	bool Call();

private:
	TIMERID _id;
	bool _callable;
	sys_clock_t _time;
	std::function<void()> _callback;
	friend class TimerUnit;
};

class TimerUnit : public Unit
{
public:
	TimerUnit() = default;
	virtual ~TimerUnit() = default;

	virtual bool Init() override final;
	virtual bool Start() override final;
	virtual void Update(intvl_t interval) override final;
	virtual void Stop() override final;
	virtual void Release() override final;

public:
	TIMERID CreateTimer(intvl_t time, std::function<void()> && callback);
	bool CallTimer(TIMERID id);
	bool RemoveTimer(TIMERID id);

private:
	void _RemoveTimer(std::shared_ptr<Timer> && timer);

private:
	std::function<bool(std::shared_ptr<Timer>, std::shared_ptr<Timer>)> _cmp = [](std::shared_ptr<Timer> left, std::shared_ptr<Timer> right) { return *left > *right; };
	std::priority_queue<std::shared_ptr<Timer>, std::vector<std::shared_ptr<Timer>>, decltype(_cmp)> _timers;
	ObjectPool<Timer> _timer_pool;
	ObjectList<Timer> _timer_list;
};

NAMESPACE_CLOSE

#endif // USRV_TIMER_UNIT_H
