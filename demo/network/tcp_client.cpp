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

class Client : public Unit
{
public:
	Client() = default;
	~Client() = default;

	virtual bool Start();
	virtual void Update(intvl_t interval);
	virtual void Stop() { };

	NETID _server_net_id = INVALID_NET_ID;
	IP _host;
	PORT _port;
};

bool Client::Start()
{
	auto server = std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get(UNITKEY::SERVER));
	_server_net_id = server->Connect(_host, _port);
	if(_server_net_id == INVALID_NET_ID)
	{
		logger::error("client connect failed.");
		return false;
	}
	return true;
}

void Client::Update(intvl_t interval)
{
	auto server = std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get(UNITKEY::SERVER));
	const char * buff = "echo check 1 check 2;";
	server->Send(_server_net_id, buff, strlen(buff));

	NETID recv_net_id = 1;
	char rec_buff[UINT16_MAX];
	uint16_t size;
	while(server->Recv(recv_net_id, rec_buff, size))
	{
		logger::info(fmt::format("recv: net_id:{} data:{}", recv_net_id, std::string(buff, size)));
	}
}

bool run_tcp_client(IP host, PORT port, int client_num)
{
	logger::init(10, 1024 * 1024);

	UnitManager::Instance()->Init(UNITKEY::COUNT, UNITKEYSTR);
	UnitManager::Instance()->Register(UNITKEY::SERVER, std::move(std::make_shared<ServerUnit>(1024, 1024, 1024 * 1024)));
	UnitManager::Instance()->Register(UNITKEY::GAME, std::move(std::make_shared<Client>()));

	auto server = std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get(UNITKEY::SERVER));
	auto game = std::dynamic_pointer_cast<Client>(UnitManager::Instance()->Get(UNITKEY::GAME));

	game->_host= host;
	game->_port = port;

	UnitManager::Instance()->Run(10);

	logger::del();

	return true;
}