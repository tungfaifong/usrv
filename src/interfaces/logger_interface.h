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
	Logger() = default;
	virtual ~Logger() = default;

public:
	void Trace(const std::string & log);
	void Debug(const std::string & log);
	void Info(const std::string & log);
	void Warn(const std::string & log);
	void Error(const std::string & log);
	void Critical(const std::string & log);
};

inline void trace(const std::string & log)
{
	Logger::Instance()->Trace(log);
}

inline void debug(const std::string & log)
{
	Logger::Instance()->Debug(log);
}

inline void info(const std::string & log)
{
	Logger::Instance()->Info(log);
}

inline void warn(const std::string & log)
{
	Logger::Instance()->Warn(log);
}

inline void error(const std::string & log)
{
	Logger::Instance()->Error(log);
}

inline void critical(const std::string & log)
{
	Logger::Instance()->Critical(log);
}

}

NAMESPACE_CLOSE

#endif // USRV_LOGGER_INTERFACE_H
