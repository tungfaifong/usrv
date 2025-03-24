// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_LOOP_HPP
#define USRV_LOOP_HPP

#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

#include "asio.hpp"

#include "util/common.h"
#include "util/time.h"

NAMESPACE_OPEN

class Loop
{
public:
	using UpdateFunc = std::function<bool()>;

	Loop(asio::io_context & io_context):_io_context(io_context),_update_timer(_io_context)  {};
	~Loop() = default;

public:
	void Init(intvl_t max_interval, UpdateFunc update, bool fixed = false)
	{
		_max_interval = max_interval * MS2NS;
		_update = update;
		_fixed = fixed;
		asio::co_spawn(_io_context, _Update(), asio::detached);
	}

	void Release()
	{
		_update = nullptr;
	}

	intvl_t Interval() { return _interval; }

private:
	asio::awaitable<void> _Update()
	{
		auto busy = false;
		auto lasy_busy = busy;
		while(true)
		{
			lasy_busy = busy;
			busy = _update();

			if(_fixed)
			{
				_interval = _max_interval;
			}
			else
			{
				if(busy)
				{
					_interval = 0;
				}
				else
				{
					if(lasy_busy)
					{
						_interval = NSINTERVAL;
					}
					else
					{
						_interval *= 2;
						_interval = _interval > _max_interval ? _max_interval : _interval;
					}
				}
			}
			
			if(_interval > 0)
			{
				_update_timer.expires_after(ns_t(_interval));
				asio::error_code ec;
				co_await _update_timer.async_wait(redirect_error(asio::use_awaitable, ec));
			}
		}
	}

private:
	asio::io_context & _io_context;
	intvl_t _interval = NSINTERVAL;
	intvl_t _max_interval;
	UpdateFunc _update;
	bool _fixed;
	asio::steady_timer _update_timer;
};

NAMESPACE_CLOSE

#endif // USRV_LOOP_HPP
