// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "unit_manager.h"
#include "units/logger_unit.h"
#include "units/lua_unit.h"
#include "units/server_unit.h"
#include "units/timer_unit.h"

#include "interfaces/logger_interface.h"
#include "interfaces/server_interface.h"
#include "interfaces/timer_interface.h"

using namespace usrv;

void SignalHandler(int signum)
{
	if(signum == SIGUSR1)
	{
		UnitManager::Instance()->SetExit(true);
	}
}

class ServerMgr : public Unit, public std::enable_shared_from_this<ServerMgr>
{
public:
	virtual bool Start();
};

bool ServerMgr::Start() {
	auto server = std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"));
	server->Listen(6666);
	return true;
}

bool run_server(intvl_t interval, size_t thread_num)
{
	signal(SIGUSR1, SignalHandler);

	UnitManager::Instance()->Init(interval);

	UnitManager::Instance()->Register("LOGGER", std::move(std::make_shared<LoggerUnit>(LoggerUnit::LEVEL::TRACE, "/logs/server.log", 1 Ki)));
	UnitManager::Instance()->Register("SERVER", std::move(std::make_shared<ServerUnit>(thread_num, 1 Ki, 1 Ki, 1 Ki)));
	UnitManager::Instance()->Register("TIMER", std::move(std::make_shared<TimerUnit>(1 Ki, 1 Ki)));
	UnitManager::Instance()->Register("LUA", std::move(std::make_shared<LuaUnit>("/main.lua")));
	UnitManager::Instance()->Register("ServerMgr", std::move(std::make_shared<ServerMgr>()));
	auto lua = std::dynamic_pointer_cast<LuaUnit>(UnitManager::Instance()->Get("LUA"));
	auto server = std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"));

	server->OnConn([&lua](NETID net_id, IP ip, PORT port) {
		lua->OnConn(net_id, ip, port);
	});

	server->OnRecv([&lua](NETID net_id, std::string && msg) {
		lua->OnRecv(net_id, msg);
	});

	server->OnDisc([&lua](NETID net_id) {
		lua->OnDisc(net_id);
	});

	UnitManager::Instance()->Run();

	return true;
}
