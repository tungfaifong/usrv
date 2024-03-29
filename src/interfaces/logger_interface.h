// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_LOGGER_INTERFACE_H
#define USRV_LOGGER_INTERFACE_H

#include "lua.hpp"

#include "units/logger_unit.h"
#include "util/common.h"
#include "unit_manager.h"

NAMESPACE_OPEN

namespace logger
{

inline void OnAbort()
{
	std::dynamic_pointer_cast<LoggerUnit>(UnitManager::Instance()->Get("LOGGER"))->OnAbort();
}

inline LoggerUnit::LEVEL Level()
{
	return std::dynamic_pointer_cast<LoggerUnit>(UnitManager::Instance()->Get("LOGGER"))->Level();
}

template<typename ... Args>
inline void log(LoggerUnit::LEVEL level, fmt::format_string<Args...> fmt, Args && ... args)
{
	std::dynamic_pointer_cast<LoggerUnit>(UnitManager::Instance()->Get("LOGGER"))->Log(level, fmt, std::forward<Args>(args)...);
}

template<typename ... Args>
inline void log(const char * file, int32_t line, const char * func, LoggerUnit::LEVEL level, fmt::format_string<Args...> fmt, Args && ... args)
{
	log(level, "[{}:{}:{}()] {}", file, line, func, fmt::format(fmt, std::forward<Args>(args)...));
}

#define LOGGER_TRACE(fmt, ...) logger::log(__FILE__, __LINE__, __func__, LoggerUnit::LEVEL::TRACE, fmt, ##__VA_ARGS__)
#define LOGGER_DEBUG(fmt, ...) logger::log(__FILE__, __LINE__, __func__, LoggerUnit::LEVEL::DEBUG, fmt, ##__VA_ARGS__)
#define LOGGER_INFO(fmt, ...) logger::log(__FILE__, __LINE__, __func__, LoggerUnit::LEVEL::INFO, fmt, ##__VA_ARGS__)
#define LOGGER_WARN(fmt, ...) logger::log(__FILE__, __LINE__, __func__, LoggerUnit::LEVEL::WARN, fmt, ##__VA_ARGS__)
#define LOGGER_ERROR(fmt, ...) logger::log(__FILE__, __LINE__, __func__, LoggerUnit::LEVEL::ERROR, fmt, ##__VA_ARGS__)
#define LOGGER_CRITICAL(fmt, ...) logger::log(__FILE__, __LINE__, __func__, LoggerUnit::LEVEL::CRITICAL, fmt, ##__VA_ARGS__)

inline uint32_t lua_level() { return (uint32_t)Level(); }
inline void lua_log(uint32_t level, uint32_t stack_level, const std::string & msg, lua_State *L) {
	lua_Debug info;
	lua_getstack(L, stack_level, &info);
	lua_getinfo(L, "Sln", &info);
	log(info.short_src, info.currentline, (info.name ? info.name : "?"), (LoggerUnit::LEVEL)level, "{}", msg);
}

}

NAMESPACE_CLOSE

#endif // USRV_LOGGER_INTERFACE_H
