// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "logger_unit.h"

#include <iostream>

#include "spdlog/spdlog.h"

#include "unit_manager.h"

NAMESPACE_OPEN

LoggerUnit::LoggerUnit(LEVEL level, std::string file_name, size_t spsc_size):_level(level), _file_name(file_name), _spsc_size(spsc_size),
	_work_guard(asio::make_work_guard(_io_context)), _loop(_io_context)
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
	return true;
}

void LoggerUnit::Stop()
{
	_io_context.stop();
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
	_LogUpdate();
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
	
	_loop.Init(_mgr->Interval(), [self = shared_from_this()](){
		return self->_LogUpdate();
	});

	_log_thread = std::thread([self = shared_from_this()](){
		self->_io_context.run();
	});
}

bool LoggerUnit::_LogUpdate()
{
	auto busy = false;
	for(auto & q : _log_queues)
	{
		LogMsg log;
		while(q.second->Pop(log))
		{
			_RealLog(log.level, std::move(log.msg));
			busy = true;
		}
	}
	return busy;
}

void LoggerUnit::_RealLog(LEVEL level, std::string && msg)
{
	_logger->log((spdlog::level::level_enum)level, std::move(msg));
}

NAMESPACE_CLOSE
