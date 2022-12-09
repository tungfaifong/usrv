// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "logger_unit.h"

#include <iostream>

#include "spdlog/spdlog.h"

#include "unit_manager.h"

NAMESPACE_OPEN

LoggerUnit::LoggerUnit(LEVEL level, std::string file_name, size_t spsc_blk_num):_level(level), _file_name(file_name), _spsc_blk_num(spsc_blk_num)
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

bool LoggerUnit::Start()
{
	_log_thread = std::thread([self = shared_from_this()](){
		self->_LogStart();
	});

	return true;
}

void LoggerUnit::Stop()
{
	_loop.SetExit(true);
	if(_log_thread.joinable())
	{
		_log_thread.join();
	}
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

void LoggerUnit::OnAbort()
{
	Stop();
	Flush();
}

void LoggerUnit::_Init()
{
	_logger = spdlog::daily_logger_st("logger", PATH_ROOT + "/" + _file_name, 0, 0);
	_logger->set_level((spdlog::level::level_enum)_level);
	_logger->flush_on((spdlog::level::level_enum)_level);
	_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%f] [%^%l%$] %v");
	
	_loop.Init(_mgr->Interval(), [self = shared_from_this()](intvl_t interval){
		return self->_LogUpdate(interval);
	});
}

void LoggerUnit::_LogStart()
{
	_loop.Run();
}

bool LoggerUnit::_LogUpdate(intvl_t interval)
{
	auto busy = false;
	for(auto & q : _log_queues)
	{
		while(!q.second->Empty())
		{
			SpscQueue::Header header;

			if(!q.second->TryPop(_log_buffer, header))
			{
				continue;
			}

			_RealLog((LEVEL)header.data16, _log_buffer, header.size);

			busy = true;
		}
	}
	return busy;
}

void LoggerUnit::_RealLog(LEVEL level, const char * log, uint16_t size)
{
	_logger->log((spdlog::level::level_enum)level, std::string_view(log, size));
}

NAMESPACE_CLOSE
