// Copyright (c) 2019-2020 TungFai Fong 

#include <iostream>

#include "unit_manager.h"
#include "units/server_unit.h"

usrv::UNITKEY CSERVERKEY;
usrv::UNITKEY CLIENTKEY;

class Client : public usrv::Unit
{
public:
	Client() = default;
	~Client() = default;

	virtual bool Start();
	virtual void Update(usrv::intvl_t interval);
	virtual void Stop() { };

	usrv::NETID _server_net_id = usrv::INVALID_NET_ID;
	usrv::IP _host;
	usrv::PORT _port;
};

bool Client::Start()
{
	auto server = std::dynamic_pointer_cast<usrv::ServerUnit>(usrv::UnitManager::Instance()->Get(CSERVERKEY));
	_server_net_id = server->Connect(_host, _port);
	return true;
}

void Client::Update(usrv::intvl_t interval)
{
	auto server = std::dynamic_pointer_cast<usrv::ServerUnit>(usrv::UnitManager::Instance()->Get(CSERVERKEY));
	const char * buff = "echo check 1 check 2;\n";
	server->Send(_server_net_id, buff, strlen(buff));

	usrv::NETID recv_net_id = 1;
	char rec_buff[UINT16_MAX];
	uint16_t size;
	while(server->Recv(recv_net_id, rec_buff, size))
	{
		printf("recv: %d %.*s", recv_net_id, size, buff);
	}
}

bool run_tcp_client(usrv::IP host, usrv::PORT port, int client_num)
{
	CSERVERKEY = usrv::UnitManager::Instance()->Register(std::move(std::make_shared<usrv::ServerUnit>(1024, 1024, 1024 * 1024)));
	CLIENTKEY = usrv::UnitManager::Instance()->Register(std::move(std::make_shared<Client>()));

	auto server = std::dynamic_pointer_cast<usrv::ServerUnit>(usrv::UnitManager::Instance()->Get(CSERVERKEY));
	auto game = std::dynamic_pointer_cast<Client>(usrv::UnitManager::Instance()->Get(CLIENTKEY));

	game->_host= host;
	game->_port = port;

	usrv::UnitManager::Instance()->Run(10);

	return true;
}