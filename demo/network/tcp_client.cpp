// Copyright (c) 2019-2020 TungFai Fong 

#include "interfaces/logger_interface.h"
#include "unit_manager.h"
#include "units/server_unit.h"

using namespace usrv;

class Client : public Unit
{
public:
	Client() = default;
	~Client() = default;

	virtual bool Start();
	virtual void Update(intvl_t interval);

	NETID _server_net_id = INVALID_NET_ID;
	IP _host;
	PORT _port;
};

bool Client::Start()
{
	auto server = std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"));
	_server_net_id = server->Connect(_host, _port, 1000);
	if(_server_net_id == INVALID_NET_ID)
	{
		LOGGER_ERROR("client connect failed.");
		return false;
	}
	return true;
}

void Client::Update(intvl_t interval)
{
	auto server = std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"));
	const char * buff = "echo check 1 check 2;";
	server->Send(_server_net_id, buff, strlen(buff));
}

bool run_tcp_client(IP host, PORT port, int client_num)
{
	UnitManager::Instance()->Init(10);
	UnitManager::Instance()->Register("LOGGER", std::move(std::make_shared<LoggerUnit>(1 Mi)));
	UnitManager::Instance()->Register("SERVER", std::move(std::make_shared<ServerUnit>(1 Ki, 1 Ki, 4 Mi)));
	UnitManager::Instance()->Register("CLIENT", std::move(std::make_shared<Client>()));

	auto server = std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"));
	auto game = std::dynamic_pointer_cast<Client>(UnitManager::Instance()->Get("CLIENT"));

	game->_host= host;
	game->_port = port;

	server->Recv([](NETID net_id, char * data, uint16_t size) {
		LOGGER_INFO("recv: net_id:{} data:{}", net_id, std::string(data, size));
	});

	UnitManager::Instance()->Run();

	return true;
}