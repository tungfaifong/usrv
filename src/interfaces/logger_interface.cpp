// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "logger_interface.h"

#include <iostream>

#include "util/time.h"

NAMESPACE_OPEN

namespace logger
{

void Logger::Init(intvl_t interval, size_t spsc_blk_num)
{
	_interval = interval;
	_log_queue = std::make_shared<SpscQueue>(spsc_blk_num);
	_log_thread = std::thread([self = shared_from_this()](){
		self->_LogLoop();
	});
}

void Logger::Del()
{
	_exit = true;
	_log_thread.join();
}

void Logger::Log(Level level, const std::string & log)
{
	SpscQueue::Header header;
	header.size = log.size();
	header.data16 = level;
	header.data32 = 0;
	_log_queue->Push(log.c_str(), header);
}

void Logger::_LogLoop()
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
			_LogUpdate(interval);
		}
		else
		{
			usleep((_interval - interval) * (CLOCKS_PER_SEC / SEC2MILLISEC));
		}
	}

	_LogUpdate(0);
}

void Logger::_LogUpdate(intvl_t interval)
{
	while(!_log_queue->Empty())
	{
		SpscQueue::Header header;

		if(!_log_queue->TryPop(_log_buffer, header))
		{
			continue;
		}

		_RealLog((Level)header.data16, _log_buffer, header.size);
	}
}

void Logger::_RealLog(Level level, const char * log, uint16_t size)
{
	static std::string prefix[Level::COUNT] = {"[trace]", "[debug]", "[info]", "[warn]", "[error]", "[critical]"};
	std::cout << prefix[level] << " " << std::string(log, size) << std::endl;
}

}

NAMESPACE_CLOSE
