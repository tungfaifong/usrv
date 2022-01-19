// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "timer_unit.h"

NAMESPACE_OPEN

// Timer
void Timer::Start(std_clock_t time, std::function<void()> && callback)
{
	_time = time;
	_callback = std::move(callback);
}

void Timer::Stop()
{
	_callback = nullptr;
}

bool Timer::Call()
{
	_callback();
	return true;
}


// TimerUnit
TimerUnit::TimerUnit(size_t tp_alloc_num, size_t ts_alloc_num): _timer_pool(tp_alloc_num),
	_timers([](std::shared_ptr<Timer> & left, std::shared_ptr<Timer> & right){ return *left < *right; }, ts_alloc_num)
{

}

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
	auto now = StdNow();
	while(!_timers.Empty())
	{
		if(_timers.Top()->_time > now)
		{
			break;
		}

		auto timer = std::move(_timers.Pop());

		timer->Call();

		_RemoveTimer(std::move(timer));
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
	auto timer = std::move(_timer_pool.Get());
	timer->Start(StdNow() + ms_t(time), std::move(callback));
	auto id = _timers.Emplace(std::move(timer));
	return id;
}

bool TimerUnit::CallTimer(TIMERID id)
{
	if(!_timers.FindByKey(id))
	{
		return false;
	}

	auto timer = std::move(_timers.PopByKey(id));

	timer->Call();

	_RemoveTimer(std::move(timer));

	return true;
}

bool TimerUnit::RemoveTimer(TIMERID id)
{
	if(!_timers.FindByKey(id))
	{
		return false;
	}

	auto timer = std::move(_timers.PopByKey(id));

	_RemoveTimer(std::move(timer));

	return true;
}

void TimerUnit::_RemoveTimer(std::shared_ptr<Timer> && timer)
{
	timer->Stop();
	_timer_pool.Put(std::move(timer));
}

NAMESPACE_CLOSE
