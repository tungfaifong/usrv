// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

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
	virtual bool Update(intvl_t interval);

	NETID _server_net_id = INVALID_NET_ID;
};

bool Client::Start()
{
	auto server = std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"));
	_server_net_id = server->Connect("127.0.0.1", 6666, 1000);
	if(_server_net_id == INVALID_NET_ID)
	{
		LOGGER_ERROR("client connect failed.");
		return false;
	}
	return true;
}

bool Client::Update(intvl_t interval)
{
	auto server = std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"));
	const char * buff = "echo check 1 check 2;";
	server->Send(_server_net_id, buff, strlen(buff));
	return true;
}

bool run_client()
{
	UnitManager::Instance()->Init(10);
	UnitManager::Instance()->Register("LOGGER", std::move(std::make_shared<LoggerUnit>(LoggerUnit::Level::TRACE, "/logs/client.log", 1 Mi)));
	UnitManager::Instance()->Register("SERVER", std::move(std::make_shared<ServerUnit>(1 Ki, 1 Ki, 4 Mi)));
	UnitManager::Instance()->Register("CLIENT", std::move(std::make_shared<Client>()));

	auto server = std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"));
	auto game = std::dynamic_pointer_cast<Client>(UnitManager::Instance()->Get("CLIENT"));

	server->OnConn([](NETID net_id, IP ip, PORT port){
		LOGGER_INFO("conn: net_id:{} ip:{} port:{}", net_id, ip, port);
	});
	server->OnRecv([](NETID net_id, char * data, uint16_t size) {
		LOGGER_INFO("recv: net_id:{} data:{}", net_id, std::string(data, size));
	});
	server->OnDisc([](NETID net_id){
		LOGGER_INFO("conn: net_id:{}", net_id);
	});

	UnitManager::Instance()->Run();

	return true;
}