// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_LOGGER_INTERFACE_H
#define USRV_LOGGER_INTERFACE_H

#include <memory>
#include <string>
#include <thread>

#include "fmt/core.h"

#include "util/common.h"
#include "util/singleton.hpp"
#include "util/spsc_queue.hpp"

NAMESPACE_OPEN

namespace logger
{

class Logger : public Singleton<Logger>, public std::enable_shared_from_this<Logger>
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

	Logger() = default;
	virtual ~Logger() = default;

public:
	void Start(intvl_t interval, size_t spsc_blk_num);
	void Stop();
	void Log(Level level, const std::string & log);

private:
	void _LogMainLoop();
	void _LogUpdate(intvl_t interval);
	void _RealLog(Level level, const char * log, uint16_t size);

private:
	bool _exit = false;
	intvl_t _interval;
	std::thread _log_thread;
	std::shared_ptr<SpscQueue> _log_queue = nullptr;
	char _log_buffer[MAX_LOG_SIZE];
};

inline void start(intvl_t interval, size_t spsc_blk_num)
{
	Logger::Instance()->Start(interval, spsc_blk_num);
}

inline void stop()
{
	Logger::Instance()->Stop();
}

inline void trace(const std::string & log)
{
	Logger::Instance()->Log(Logger::Level::TRACE, log);
}

inline void debug(const std::string & log)
{
	Logger::Instance()->Log(Logger::Level::DEBUG, log);
}

inline void info(const std::string & log)
{
	Logger::Instance()->Log(Logger::Level::INFO, log);
}

inline void warn(const std::string & log)
{
	Logger::Instance()->Log(Logger::Level::WARN, log);
}

inline void error(const std::string & log)
{
	Logger::Instance()->Log(Logger::Level::ERROR, log);
}

inline void critical(const std::string & log)
{
	Logger::Instance()->Log(Logger::Level::CRITICAL, log);
}

}

NAMESPACE_CLOSE

#endif // USRV_LOGGER_INTERFACE_H
