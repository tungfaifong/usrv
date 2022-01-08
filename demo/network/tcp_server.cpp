// Copyright (c) 2019-2020 TungFai Fong 

#include "interfaces/logger_interface.h"
#include "unit_manager.h"
#include "units/server_unit.h"

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

class Game : public usrv::Unit
{
public:
	Game() = default;
	~Game() = default;

	virtual bool Start() { return true; };
	virtual void Update(usrv::intvl_t interval);
	virtual void Stop() { };
};

void Game::Update(usrv::intvl_t interval)
{
	auto server = std::dynamic_pointer_cast<usrv::ServerUnit>(usrv::UnitManager::Instance()->Get(UNITKEY::SERVER));

	usrv::NETID net_id;
	char buff[UINT16_MAX];
	uint16_t size;
	while(server->Recv(net_id, buff, size))
	{
		usrv::logger::info(fmt::format("recv: net_id:{} data:{}", net_id, std::string(buff, size)));
		server->Send(net_id, buff, size);
	}
}

bool run_tcp_server(usrv::PORT port)
{
	usrv::UnitManager::Instance()->Init(UNITKEY::COUNT, UNITKEYSTR);
	usrv::UnitManager::Instance()->Register(UNITKEY::SERVER, std::move(std::make_shared<usrv::ServerUnit>(1024, 1024, 1024 * 1024)));
	usrv::UnitManager::Instance()->Register(UNITKEY::GAME, std::move(std::make_shared<Game>()));

	auto server = std::dynamic_pointer_cast<usrv::ServerUnit>(usrv::UnitManager::Instance()->Get(UNITKEY::SERVER));

	server->Listen(port);

	usrv::UnitManager::Instance()->Run(10);

	return true;
}