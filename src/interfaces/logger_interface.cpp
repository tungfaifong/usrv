// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "logger_interface.h"

#include <iostream>

NAMESPACE_OPEN

namespace logger
{

void Logger::Log(Level level, const std::string & log)
{
	static std::string prefix[Level::COUNT] = {"[trace]", "[debug]", "[info]", "[warn]", "[error]", "[critical]"};
	std::cout << prefix[level] << " " << log << std::endl;
}

}

NAMESPACE_CLOSE
