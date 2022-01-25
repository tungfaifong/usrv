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
inline void log(LoggerUnit::Level level, fmt::format_string<Args...> fmt, Args && ... args)
{
	std::dynamic_pointer_cast<LoggerUnit>(UnitManager::Instance()->Get("LOGGER"))->Log(level, fmt, std::forward<Args>(args)...);
}

template<typename T>
inline void log(LoggerUnit::Level level, const T & msg)
{
	log(level, "{}", msg);
}

template<typename ... Args>
inline void log(const char * file, int32_t line, const char * func, LoggerUnit::Level level, fmt::format_string<Args...> fmt, Args && ... args)
{
	log(level, "[{}:{}:{}()] {}", file, line, func, fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename T>
inline void log(const char * file, int32_t line, const char * func, LoggerUnit::Level level, const T & msg)
{
	log(file, line, func, level, "{}", msg);
}

inline void flush()
{
	std::dynamic_pointer_cast<LoggerUnit>(UnitManager::Instance()->Get("LOGGER"))->Flush();
}

#define LOGGER_TRACE(...) logger::log(__FILE__, __LINE__, __FUNCTION__, LoggerUnit::Level::TRACE, __VA_ARGS__)
#define LOGGER_DEBUG(...) logger::log(__FILE__, __LINE__, __FUNCTION__, LoggerUnit::Level::DEBUG, __VA_ARGS__)
#define LOGGER_INFO(...) logger::log(__FILE__, __LINE__, __FUNCTION__, LoggerUnit::Level::INFO, __VA_ARGS__)
#define LOGGER_WARN(...) logger::log(__FILE__, __LINE__, __FUNCTION__, LoggerUnit::Level::WARN, __VA_ARGS__)
#define LOGGER_ERROR(...) logger::log(__FILE__, __LINE__, __FUNCTION__, LoggerUnit::Level::ERROR, __VA_ARGS__)
#define LOGGER_CRITICAL(...) logger::log(__FILE__, __LINE__, __FUNCTION__, LoggerUnit::Level::CRITICAL, __VA_ARGS__)
#define LOGGER_FLUSH() logger::flush()

inline void lua_log(uint8_t level, const std::string & msg) { log((LoggerUnit::Level)level, msg); };

}

NAMESPACE_CLOSE

#endif // USRV_LOGGER_INTERFACE_H
