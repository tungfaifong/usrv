// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_LOGGER_UNIT_H
#define USRV_LOGGER_UNIT_H

#include <thread>

#include "fmt/core.h"

#include "components/loop.hpp"
#include "unit.h"
#include "util/common.h"
#include "util/spsc_queue.hpp"

NAMESPACE_OPEN

class LoggerUnit : public Unit, public std::enable_shared_from_this<LoggerUnit>
{
public:
	static const uint16_t MAX_LOG_SIZE = 2048;

	enum Level
	{
		TRACE = 0,
		DEBUG,
		INFO,
		WARN,
		ERROR,
		CRITICAL,
		COUNT,
	};

	LoggerUnit(size_t spsc_blk_num);
	virtual ~LoggerUnit() = default;

	virtual bool Init() override final;
	virtual bool Start() override final;
	virtual void Update(intvl_t interval) override final;
	virtual void Stop() override final;
	virtual void Release() override final;

public:
	template<typename ... Args> void Log(Level level, fmt::format_string<Args...> fmt, Args && ... args);

private:
	void _LogStart();
	void _LogUpdate(intvl_t interval);
	void _RealLog(Level level, const char * log, uint16_t size);

private:
	Loop _loop;
	std::thread _log_thread;
	SpscQueue _log_queue;
	char _log_buffer[MAX_LOG_SIZE];
};

template<typename ... Args> void LoggerUnit::Log(Level level, fmt::format_string<Args...> fmt, Args && ... args)
{
	std::string log = fmt::format(fmt, std::forward<Args>(args)...);

	SpscQueue::Header header;
	header.size = log.size();
	header.data16 = level;
	header.data32 = 0;

	_log_queue.Push(log.c_str(), header);
}

NAMESPACE_CLOSE

#endif // USRV_LOGGER_UNIT_H
