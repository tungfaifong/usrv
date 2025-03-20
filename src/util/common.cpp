// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "common.h"

#include <signal.h>

#include "interfaces/logger_interface.h"

NAMESPACE_OPEN

IP DEFAULT_IP {""};
std::string PATH_ROOT = getcwd(NULL, 1024);

void SignalHandler(int signo)
{
	switch(signo)
	{
		case SIGILL:
		case SIGTRAP:
		case SIGABRT:
		case SIGBUS:
		case SIGFPE:
		case SIGSEGV:
		case SIGSYS:
		{
			signal(signo, SIG_DFL);
			logger::OnAbort();
		}
		break;
		case SIGUSR1:
			UnitManager::Instance()->SetExit(true);
			break;
		default:
			break;
	}
}

void SignalInit()
{
	signal(SIGILL, SignalHandler);
	signal(SIGTRAP, SignalHandler);
	signal(SIGABRT, SignalHandler);
	signal(SIGBUS, SignalHandler);
	signal(SIGFPE, SignalHandler);
	signal(SIGSEGV, SignalHandler);
	signal(SIGSEGV, SignalHandler);
	signal(SIGUSR1, SignalHandler);
}

NAMESPACE_CLOSE
