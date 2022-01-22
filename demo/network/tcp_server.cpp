// Copyright (c) 2019-2020 TungFai Fong 

#include "interfaces/logger_interface.h"
#include "unit_manager.h"
#include "units/logger_unit.h"
#include "units/server_unit.h"
#include "util/common.h"

using namespace usrv;

void SignalHandler(int signum)
{
	if(signum == SIGUSR1)
	{
		UnitManager::Instance()->SetExit(true);
	}
}

bool run_tcp_server(PORT port)
{
	signal(SIGUSR1, SignalHandler);

	UnitManager::Instance()->Init(10);
	UnitManager::Instance()->Register("LOGGER", std::move(std::make_shared<LoggerUnit>(1 Mi)));
	UnitManager::Instance()->Register("SERVER", std::move(std::make_shared<ServerUnit>(1 Ki, 1 Ki, 4 Mi)));

	auto server = std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"));
	server->Listen(port);

	server->Recv([&server](NETID net_id, char * data, uint16_t size) {
		logger::info("recv: net_id:{} data:{}", net_id, std::string_view(data, size));
		server->Send(net_id, data, size);
	});

	UnitManager::Instance()->Run();

	return true;
}