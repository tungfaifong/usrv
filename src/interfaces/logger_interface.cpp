// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "logger_interface.h"

NAMESPACE_OPEN

namespace logger
{

void Logger::Trace(const std::string & log)
{
	printf("[trace] %s \n", log.c_str());
}

void Logger::Debug(const std::string & log)
{
	printf("[debug] %s \n", log.c_str());
}

void Logger::Info(const std::string & log)
{
	printf("[info] %s \n", log.c_str());
}

void Logger::Warn(const std::string & log)
{
	printf("[warn] %s \n", log.c_str());
}

void Logger::Error(const std::string & log)
{
	printf("[error] %s \n", log.c_str());
}

void Logger::Critical(const std::string & log)
{
	printf("[critical] %s \n", log.c_str());
}

}

NAMESPACE_CLOSE
