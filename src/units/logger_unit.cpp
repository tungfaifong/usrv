// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "logger_unit.h"

#include <iostream>

#include "unit_manager.h"

NAMESPACE_OPEN

LoggerUnit::LoggerUnit(size_t spsc_blk_num): _log_queue(spsc_blk_num)
{
	
}

bool LoggerUnit::Init()
{
	_loop.Init(_mgr->Interval(), [self = shared_from_this()](intvl_t interval){
		self->_LogUpdate(interval);
	});

	return true;
}

bool LoggerUnit::Start()
{
	_log_thread = std::thread([self = shared_from_this()](){
		self->_LogStart();
	});

	return true;
}

void LoggerUnit::Update(intvl_t interval)
{

}

void LoggerUnit::Stop()
{
	_loop.SetExit(true);
	_log_thread.join();
}

void LoggerUnit::Release()
{
	_LogUpdate(0);
}

void LoggerUnit::Log(Level level, const std::string & log)
{
	SpscQueue::Header header;
	header.size = log.size();
	header.data16 = level;
	header.data32 = 0;
	_log_queue.Push(log.c_str(), header);
}

void LoggerUnit::_LogStart()
{
	_loop.Run();
}

void LoggerUnit::_LogUpdate(intvl_t interval)
{
	while(!_log_queue.Empty())
	{
		SpscQueue::Header header;

		if(!_log_queue.TryPop(_log_buffer, header))
		{
			continue;
		}

		_RealLog((Level)header.data16, _log_buffer, header.size);
	}
}

void LoggerUnit::_RealLog(Level level, const char * log, uint16_t size)
{
	static std::string prefix[Level::COUNT] = {"[trace]", "[debug]", "[info]", "[warn]", "[error]", "[critical]"};
	std::cout << prefix[level] << " " << std::string(log, size) << std::endl;
}

NAMESPACE_CLOSE
