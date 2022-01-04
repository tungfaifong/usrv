// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "timer_unit.h"

#include "util/time.h"

namespace usrv
{
	
TimerUnit::TimerUnit()
{
}

TimerUnit::~TimerUnit()
{
}

bool TimerUnit::Start()
{
	return true;
}

void TimerUnit::Update(intvl_t interval)
{
	auto now = ClockMS();
	while(!_timers.empty())
	{
		auto timer = _timers.top();
		if(timer._time > now)
		{
			break;
		}

		timer._callback();
		_timers.pop();
	}
}

void TimerUnit::Stop()
{

}

void TimerUnit::CreateTimer(intvl_t time, std::function<void()> && callback)
{
	Timer t(ClockMS() + time, std::move(callback));
	_timers.emplace(std::move(t));
}

}
