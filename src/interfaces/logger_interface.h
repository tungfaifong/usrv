// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_LOGGER_INTERFACE_H
#define USRV_LOGGER_INTERFACE_H

#include "unit_manager.h"
#include "units/logger_unit.h"
#include "util/common.h"

NAMESPACE_OPEN

namespace logger
{

template<typename ... Args>
inline void trace(fmt::format_string<Args...> fmt, Args && ... args)
{
	std::dynamic_pointer_cast<LoggerUnit>(UnitManager::Instance()->Get("LOGGER"))->Log(LoggerUnit::Level::TRACE, fmt, std::forward<Args>(args)...);
}

template<typename ... Args>
inline void debug(fmt::format_string<Args...> fmt, Args && ... args)
{
	std::dynamic_pointer_cast<LoggerUnit>(UnitManager::Instance()->Get("LOGGER"))->Log(LoggerUnit::Level::DEBUG, fmt, std::forward<Args>(args)...);
}

template<typename ... Args>
inline void info(fmt::format_string<Args...> fmt, Args && ... args)
{
	std::dynamic_pointer_cast<LoggerUnit>(UnitManager::Instance()->Get("LOGGER"))->Log(LoggerUnit::Level::INFO, fmt, std::forward<Args>(args)...);
}

template<typename ... Args>
inline void warn(fmt::format_string<Args...> fmt, Args && ... args)
{
	std::dynamic_pointer_cast<LoggerUnit>(UnitManager::Instance()->Get("LOGGER"))->Log(LoggerUnit::Level::WARN, fmt, std::forward<Args>(args)...);
}

template<typename ... Args>
inline void error(fmt::format_string<Args...> fmt, Args && ... args)
{
	std::dynamic_pointer_cast<LoggerUnit>(UnitManager::Instance()->Get("LOGGER"))->Log(LoggerUnit::Level::ERROR, fmt, std::forward<Args>(args)...);
}

template<typename ... Args>
inline void critical(fmt::format_string<Args...> fmt, Args && ... args)
{
	std::dynamic_pointer_cast<LoggerUnit>(UnitManager::Instance()->Get("LOGGER"))->Log(LoggerUnit::Level::CRITICAL, fmt, std::forward<Args>(args)...);
}

inline void trace_l(const char * log) { trace("{}", log); }
inline void debug_l(const char * log) { debug("{}", log); }
inline void info_l(const char * log) { info("{}", log); }
inline void warn_l(const char * log) { warn("{}", log); }
inline void error_l(const char * log) { error("{}", log); }
inline void critical_l(const char * log) { critical("{}", log); }

inline void flush()
{
	std::dynamic_pointer_cast<LoggerUnit>(UnitManager::Instance()->Get("LOGGER"))->Flush();
}

}

NAMESPACE_CLOSE

#endif // USRV_LOGGER_INTERFACE_H
