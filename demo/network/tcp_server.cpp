// Copyright (c) 2019-2020 TungFai Fong 

#include "interfaces/logger_interface.h"
#include "unit_manager.h"
#include "units/server_unit.h"

using namespace usrv;

enum UNITKEY
{
	SERVER = 0,
	GAME,
	COUNT,
};

const std::string UNITKEYSTR[UNITKEY::COUNT] = 
{
	"SERVER",
	"GAME"
};

class Game : public Unit
{
public:
	Game() = default;
	~Game() = default;

	virtual bool Start() { return true; };
	virtual void Update(intvl_t interval);
	virtual void Stop() { };
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
	auto server = std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get(UNITKEY::SERVER));

	NETID net_id;
	char buff[UINT16_MAX];
	uint16_t size;
	while(server->Recv(net_id, buff, size))
	{
		// logger::info(fmt::format("recv: net_id:{} data:{}", net_id, std::string(buff, size)));
		server->Send(net_id, buff, size);
	}
}

bool run_tcp_server(PORT port)
{
	signal(SIGUSR1, SignalHandler);

	UnitManager::Instance()->Init(UNITKEY::COUNT, UNITKEYSTR);
	UnitManager::Instance()->Register(UNITKEY::SERVER, std::move(std::make_shared<ServerUnit>(1024, 1024, 1024 * 1024)));
	UnitManager::Instance()->Register(UNITKEY::GAME, std::move(std::make_shared<Game>()));

	auto server = std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get(UNITKEY::SERVER));

	server->Listen(port);

	UnitManager::Instance()->Run(10);

	return true;
}