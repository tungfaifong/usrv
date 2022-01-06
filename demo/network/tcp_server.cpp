// Copyright (c) 2019-2020 TungFai Fong 

#include <iostream>

#include "unit_manager.h"
#include "units/server_unit.h"

usrv::UNITKEY SSERVERKEY;
usrv::UNITKEY GAMEKEY;

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
	auto server = std::dynamic_pointer_cast<usrv::ServerUnit>(usrv::UnitManager::Instance()->Get(SSERVERKEY));

	usrv::NETID net_id;
	char buff[UINT16_MAX];
	uint16_t size;
	while(server->Recv(net_id, buff, size))
	{
		printf("recv: %d %.*s", net_id, size, buff);
		server->Send(net_id, buff, size);
	}
}

bool run_tcp_server(usrv::PORT port)
{
	SSERVERKEY = usrv::UnitManager::Instance()->Register(std::move(std::make_shared<usrv::ServerUnit>(1024, 1024, 1024 * 1024)));
	usrv::UnitManager::Instance()->Register(std::move(std::make_shared<Game>()));

	auto server = std::dynamic_pointer_cast<usrv::ServerUnit>(usrv::UnitManager::Instance()->Get(SSERVERKEY));

	server->Listen(port);

	usrv::UnitManager::Instance()->Run(10);

	return true;
}