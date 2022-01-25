// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "logger_unit.h"

#include <iostream>

#include "spdlog/spdlog.h"

#include "unit_manager.h"

NAMESPACE_OPEN

LoggerUnit::LoggerUnit(size_t spsc_blk_num): _spsc_blk_num(spsc_blk_num)
{

}

LoggerUnit::~LoggerUnit()
{
	Flush();
}

void LoggerUnit::OnRegister(const std::shared_ptr<UnitManager> & mgr)
{
	Unit::OnRegister(mgr);
	_Init();
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

void LoggerUnit::_Init()
{
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%f] [%^%l%$] %v");
	spdlog::set_level(spdlog::level::trace);
	_loop.Init(_mgr->Interval(), [self = shared_from_this()](intvl_t interval){
		self->_LogUpdate(interval);
	});
	_log_thread = std::thread([self = shared_from_this()](){
		self->_LogStart();
	});
}

void LoggerUnit::_LogStart()
{
	_loop.Run();
}

void LoggerUnit::_LogUpdate(intvl_t interval)
{
	for(auto & q : _log_queues)
	{
		while(!q.second->Empty())
		{
			SpscQueue::Header header;

			if(!q.second->TryPop(_log_buffer, header))
			{
				continue;
			}

			_RealLog((Level)header.data16, _log_buffer, header.size);
		}
	}
	
}

void LoggerUnit::_RealLog(Level level, const char * log, uint16_t size)
{
	spdlog::log((spdlog::level::level_enum)level, std::string_view(log, size));
}

NAMESPACE_CLOSE
