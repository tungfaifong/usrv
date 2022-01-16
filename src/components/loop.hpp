// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_LOOP_HPP
#define USRV_LOOP_HPP

#include <functional>

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
		auto start = SysNow();
		auto now = start;
		auto interval = Ns2Ms(now - start);
		while (!_exit)
		{
			now = SysNow();
			interval = Ns2Ms(now - start);
			if (interval >= _interval)
			{
				start = now;
				_update_func(interval);
			}
			else
			{
				usleep((_interval - interval) * (CLOCKS_PER_SEC / SEC2MILLISEC));
			}
		}
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
