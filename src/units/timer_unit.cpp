// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "timer_unit.h"

NAMESPACE_OPEN

// Timer
void Timer::Start(TIMERID id, sys_clock_t time, std::function<void()> && callback)
{
	_id = id;
	_callable = true;
	_time = time;
	_callback = std::move(callback);
}

void Timer::Stop()
{
	_id = INVALID_TIMER_ID;
	_callable = false;
	_callback = nullptr;
}

bool Timer::Call()
{
	if(!_callable)
	{
		return false;
	}

	_callback();

	return true;
}


// TimerUnit
bool TimerUnit::Init()
{
	return true;
}

bool TimerUnit::Start()
{
	return true;
}

void TimerUnit::Update(intvl_t interval)
{
	auto now = SysNow();
	while(!_timers.empty())
	{
		auto timer = _timers.top();
		if(timer->_time > now)
		{
			break;
		}

		timer->Call();

		_RemoveTimer(std::move(timer));

		_timers.pop();
	}
}

void TimerUnit::Stop()
{

}

void TimerUnit::Release()
{

}

TIMERID TimerUnit::CreateTimer(intvl_t time, std::function<void()> && callback)
{
	auto timer_id = _timer_list.Insert(std::move(_timer_pool.Get()));
	auto timer = _timer_list[timer_id];
	timer->Start(timer_id, SysNow() + ms_t(time), std::move(callback));
	_timers.emplace(timer);
	return timer_id;
}

bool TimerUnit::CallTimer(TIMERID id)
{
	auto timer = _timer_list[id];
	if(!timer)
	{
		return false;
	}

	if(!timer->Call())
	{
		return false;
	}

	_RemoveTimer(std::move(timer));

	return true;
}

bool TimerUnit::RemoveTimer(TIMERID id)
{
	auto timer = _timer_list[id];
	if(!timer)
	{
		return false;
	}

	_RemoveTimer(std::move(timer));

	return true;
}

void TimerUnit::_RemoveTimer(std::shared_ptr<Timer> && timer)
{
	timer->Stop();
	_timer_list.Erase(timer->_id);
	_timer_pool.Put(std::move(timer));
}

NAMESPACE_CLOSE
