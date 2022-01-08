// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_LOGGER_INTERFACE_H
#define USRV_LOGGER_INTERFACE_H

#include "fmt/core.h"

#include "util/common.h"
#include "util/singleton.hpp"

NAMESPACE_OPEN

namespace logger
{

class Logger : public Singleton<Logger>
{
public:
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
	void Log(Level level, const std::string & log);
};

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
