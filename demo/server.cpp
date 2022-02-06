// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "unit_manager.h"
#include "units/logger_unit.h"
#include "units/lua_unit.h"
#include "units/server_unit.h"
#include "units/timer_unit.h"

using namespace usrv;

void SignalHandler(int signum)
{
	if(signum == SIGUSR1)
	{
		UnitManager::Instance()->SetExit(true);
	}
}

bool run_server(intvl_t interval)
{
	signal(SIGUSR1, SignalHandler);

	UnitManager::Instance()->Init(interval);

	UnitManager::Instance()->Register("LOGGER", std::move(std::make_shared<LoggerUnit>(LoggerUnit::Level::TRACE, "/logs/server.log", 1 Mi)));
	UnitManager::Instance()->Register("SERVER", std::move(std::make_shared<ServerUnit>(1 Ki, 1 Ki, 1 Mi)));
	UnitManager::Instance()->Register("TIMER", std::move(std::make_shared<TimerUnit>(1 Ki, 1 Ki)));
	UnitManager::Instance()->Register("LUA", std::move(std::make_shared<LuaUnit>(usrv::PATH_ROOT + "/main.lua")));

	auto server = std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"));
	server->Listen(6666);

	auto lua = std::dynamic_pointer_cast<LuaUnit>(UnitManager::Instance()->Get("LUA"));

	server->OnConn([&lua](NETID net_id, IP ip, PORT port) {
		lua->OnConn(net_id, ip, port);
	});

	server->OnRecv([&lua](NETID net_id, char * data, uint16_t size) {
		lua->OnRecv(net_id, data, size);
	});

	server->OnDisc([&lua](NETID net_id) {
		lua->OnDisc(net_id);
	});

	UnitManager::Instance()->Run();

	return true;
}
