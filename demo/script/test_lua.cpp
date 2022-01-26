// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "unit_manager.h"
#include "units/logger_unit.h"
#include "units/lua_unit.h"
#include "units/server_unit.h"
#include "units/timer_unit.h"

using namespace usrv;

bool run_lua()
{
	UnitManager::Instance()->Init(10);

	UnitManager::Instance()->Register("LOGGER", std::move(std::make_shared<LoggerUnit>(LoggerUnit::Level::TRACE, "/logs/lua.log", 1 Mi)));
	UnitManager::Instance()->Register("SERVER", std::move(std::make_shared<ServerUnit>(1 Ki, 1 Ki, 4 Mi)));
	UnitManager::Instance()->Register("TIMER", std::move(std::make_shared<TimerUnit>(1 Ki, 1 Ki)));
	UnitManager::Instance()->Register("LUA", std::move(std::make_shared<LuaUnit>(usrv::PATH_ROOT + "/demo/script/main.lua")));

	auto server = std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"));
	server->Listen(6666);

	auto lua = std::dynamic_pointer_cast<LuaUnit>(UnitManager::Instance()->Get("LUA"));

	server->Recv([&lua](NETID net_id, char * data, uint16_t size) {
		lua->OnRecvFunc(net_id, data, size);
	});

	UnitManager::Instance()->Run();

	return true;
}
