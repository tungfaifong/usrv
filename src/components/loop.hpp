// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_LOOP_HPP
#define USRV_LOOP_HPP

#include <functional>
#include <thread>

#include "util/common.h"
#include "util/time.h"

NAMESPACE_OPEN

class Loop
{
public:
	using UpdateFunc = std::function<void(intvl_t)>;

	Loop() = default;
	~Loop() = default;

public:
	void Init(intvl_t interval, UpdateFunc update_func)
	{
		_interval = interval;
		_update_func = update_func;
	}

	void Run()
	{
		auto start = StdNow();
		auto now = start;
		auto interval = Ns2Ms(now - start);
		while (!_exit)
		{
			now = StdNow();
			interval = Ns2Ms(now - start);
			if (interval >= _interval)
			{
				start = now;
				_update_func(interval);
			}
			else
			{
				std::this_thread::sleep_for(ms_t(_interval - interval));
			}
		}
	}

	void Release()
	{
		_update_func = nullptr;
	}

	void SetExit(bool exit) { _exit = exit; }

	intvl_t Interval() { return _interval; }

private:
	bool _exit = false;
	intvl_t _interval;
	UpdateFunc _update_func;
};

NAMESPACE_CLOSE

#endif // USRV_LOOP_HPP
