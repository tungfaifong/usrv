// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_LOGGER_UNIT_H
#define USRV_LOGGER_UNIT_H

#include <map>
#include <thread>

#include "spdlog/sinks/daily_file_sink.h"
#include "fmt/core.h"

#include "components/loop.hpp"
#include "util/spsc_queue.hpp"
#include "unit.h"

NAMESPACE_OPEN

class LogMsg;

class LoggerUnit : public Unit, public std::enable_shared_from_this<LoggerUnit>
{
public:
	static constexpr uint16_t MAX_LOG_SIZE = UINT16_MAX;
	enum class LEVEL
	{
		TRACE = 0,
		DEBUG,
		INFO,
		WARN,
		ERROR,
		CRITICAL,
		OFF,
		COUNT,
	};

	LoggerUnit(LEVEL level, std::string file_name, size_t spsc_size);
	virtual ~LoggerUnit();

	virtual void OnRegister(const std::shared_ptr<UnitManager> & mgr) override final;
	virtual void Stop() override final;
	virtual void Release() override final;

public:
	template<typename ... Args> void Log(LEVEL level, fmt::format_string<Args...> fmt, Args && ... args);
	void Flush();
	void OnAbort();
	LEVEL Level() { return _level; }

private:
	void _Init();
	void _LogStart();
	bool _LogUpdate();
	void _RealLog(LEVEL level, std::string && msg);

private:
	friend class LogMsgBase;

	LEVEL _level;
	std::string _file_name;
	size_t _spsc_size;
	asio::io_context _io_context;
	asio::executor_work_guard<asio::io_context::executor_type> _work_guard;
	std::thread _log_thread;
	Loop _loop;
	std::unordered_map<std::thread::id, std::shared_ptr<SpscQueue<LogMsg>>> _log_queues;
	std::shared_ptr<spdlog::logger> _logger;
};

class LogMsg
{
public:
	LogMsg() = default;
	LogMsg(LoggerUnit::LEVEL level, std::string && msg)
			: level(level), msg(std::move(msg)) {}

	LogMsg(LogMsg&&) = default;
	LogMsg& operator=(LogMsg&&) = default;

	LoggerUnit::LEVEL level;
	std::string msg;
};

template<typename ... Args> void LoggerUnit::Log(LEVEL level, fmt::format_string<Args...> fmt, Args && ... args)
{
	if(level < _level)
	{
		return;
	}

	auto tid = std::this_thread::get_id();
	if(_log_queues.find(tid) == _log_queues.end())
	{
		_log_queues[tid] = std::move(std::make_shared<SpscQueue<LogMsg>>(_spsc_size));
	}

	while(!_log_queues[tid]->Push(level, std::move(fmt::format(fmt, std::forward<Args>(args)...)))) {}
}

NAMESPACE_CLOSE

#endif // USRV_LOGGER_UNIT_H
