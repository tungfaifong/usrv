// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_LOGGER_INTERFACE_H
#define USRV_LOGGER_INTERFACE_H

#include "fmt/core.h"

#include "unit_manager.h"
#include "units/logger_unit.h"
#include "util/common.h"

NAMESPACE_OPEN

namespace logger
{

inline void trace(const std::string & log)
{
	std::dynamic_pointer_cast<LoggerUnit>(UnitManager::Instance()->Get("LOGGER"))->Log(LoggerUnit::Level::TRACE, log);
}

inline void debug(const std::string & log)
{
	std::dynamic_pointer_cast<LoggerUnit>(UnitManager::Instance()->Get("LOGGER"))->Log(LoggerUnit::Level::DEBUG, log);
}

inline void info(const std::string & log)
{
	std::dynamic_pointer_cast<LoggerUnit>(UnitManager::Instance()->Get("LOGGER"))->Log(LoggerUnit::Level::INFO, log);
}

inline void warn(const std::string & log)
{
	std::dynamic_pointer_cast<LoggerUnit>(UnitManager::Instance()->Get("LOGGER"))->Log(LoggerUnit::Level::WARN, log);
}

inline void error(const std::string & log)
{
	std::dynamic_pointer_cast<LoggerUnit>(UnitManager::Instance()->Get("LOGGER"))->Log(LoggerUnit::Level::ERROR, log);
}

inline void critical(const std::string & log)
{
	std::dynamic_pointer_cast<LoggerUnit>(UnitManager::Instance()->Get("LOGGER"))->Log(LoggerUnit::Level::CRITICAL, log);
}

}

NAMESPACE_CLOSE

#endif // USRV_LOGGER_INTERFACE_H
