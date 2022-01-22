// Copyright (c) 2019-2020 TungFai Fong 

#include "interfaces/logger_interface.hpp"
#include "unit_manager.h"
#include "units/logger_unit.h"
#include "units/server_unit.h"
#include "util/common.h"

using namespace usrv;

class Game : public Unit
{
public:
	Game() = default;
	~Game() = default;

	virtual void Update(intvl_t interval);
};

void SignalHandler(int signum)
{
	if(signum == SIGUSR1)
	{
		UnitManager::Instance()->SetExit(true);
	}
}

void Game::Update(intvl_t interval)
{
	auto server = std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"));

	NETID net_id;
	char buff[UINT16_MAX];
	uint16_t size;
	while(server->Recv(net_id, buff, size))
	{
		logger::info("recv: net_id:{} data:{}", net_id, std::string_view(buff, size));
		server->Send(net_id, buff, size);
	}
}

bool run_tcp_server(PORT port)
{
	signal(SIGUSR1, SignalHandler);

	UnitManager::Instance()->Init(10);
	UnitManager::Instance()->Register("LOGGER", std::move(std::make_shared<LoggerUnit>(1 Mi)));
	UnitManager::Instance()->Register("SERVER", std::move(std::make_shared<ServerUnit>(1 Ki, 1 Ki, 4 Mi)));
	UnitManager::Instance()->Register("GAME", std::move(std::make_shared<Game>()));

	auto server = std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"));
	server->Listen(port);

	UnitManager::Instance()->Run();

	return true;
}