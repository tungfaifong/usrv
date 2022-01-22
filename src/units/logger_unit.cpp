// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "logger_unit.h"

#include <iostream>

#include "unit_manager.h"

NAMESPACE_OPEN

LoggerUnit::LoggerUnit(size_t spsc_blk_num): _log_queue(spsc_blk_num)
{

}

LoggerUnit::~LoggerUnit()
{
	Flush();
}

void LoggerUnit::OnRegister(const std::shared_ptr<UnitManager> & mgr)
{
	Unit::OnRegister(mgr);
	_loop.Init(_mgr->Interval(), [self = shared_from_this()](intvl_t interval){
		self->_LogUpdate(interval);
	});
	_log_thread = std::thread([self = shared_from_this()](){
		self->_LogStart();
	});
}

void LoggerUnit::Stop()
{
	_loop.SetExit(true);
	_log_thread.join();
}

void LoggerUnit::Release()
{
	_loop.Release();
	Unit::Release();
}

void LoggerUnit::Flush()
{
	_LogUpdate(0);
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
	std::cout << prefix[level] << " " << std::string_view(log, size) << std::endl;
}

NAMESPACE_CLOSE
