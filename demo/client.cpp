// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "interfaces/logger_interface.h"
#include "interfaces/server_interface.h"
#include "interfaces/timer_interface.h"
#include "unit_manager.h"
#include "units/server_unit.h"

using namespace usrv;

class Client;

uint32_t g_TotalCnt = 0;
std::unordered_map<NETID, std::shared_ptr<Client>> g_Clients;

class Client : public Unit, public std::enable_shared_from_this<Client>
{
public:
	Client(uint32_t req_num):_req_num(req_num) {}
	~Client() = default;

	virtual bool Start();
	void OnRecv(NETID net_id, char * data, uint16_t size);
	void Send();

	NETID _server_net_id = INVALID_NET_ID;
	uint32_t _req_num = 0;
};

bool Client::Start()
{
	_server_net_id = server::Connect("127.0.0.1", 6666, 1000);
	if(_server_net_id == INVALID_NET_ID)
	{
		LOGGER_ERROR("client connect failed.");
		return false;
	}
	g_Clients[_server_net_id] = shared_from_this();
	for(uint32_t i = 0; i < _req_num; ++i)
	{
		Send();
	}
	return true;
}

void Client::OnRecv(NETID net_id, char * data, uint16_t size)
{
	++g_TotalCnt;
	Send();
}

void Client::Send()
{
	const char * buff = "echo check 1 check 2;";
	server::Send(_server_net_id, buff, strlen(buff));
}

bool run_client(uint32_t client_num, uint32_t req_num, uint32_t time)
{
	UnitManager::Instance()->Init(10);
	UnitManager::Instance()->Register("LOGGER", std::move(std::make_shared<LoggerUnit>(LoggerUnit::LEVEL::TRACE, "/logs/client.log", 1 Mi)));
	UnitManager::Instance()->Register("TIMER", std::move(std::make_shared<TimerUnit>(1 Ki, 1 Ki)));
	UnitManager::Instance()->Register("SERVER", std::move(std::make_shared<ServerUnit>(1 Ki, 1 Ki, 4 Mi)));

	for(uint32_t i = 0; i < client_num; ++i)
	{
		auto client_key = "CLIENT#"+std::to_string(i+1);
		if(!UnitManager::Instance()->Register(client_key.c_str(), std::move(std::make_shared<Client>(req_num))))
		{
			LOGGER_ERROR("key:{} error", client_key);
		}
	}

	auto server = std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"));
	server->OnConn([](NETID net_id, IP ip, PORT port){
		LOGGER_INFO("conn: net_id:{} ip:{} port:{}", net_id, ip, port);
	});
	server->OnRecv([](NETID net_id, char * data, uint16_t size) {
		auto client = g_Clients.find(net_id);
		if(client != g_Clients.end())
		{
			client->second->OnRecv(net_id, data, size);
		}
	});
	server->OnDisc([](NETID net_id){
		LOGGER_INFO("conn: net_id:{}", net_id);
	});

	timer::CreateTimer(time, [](){UnitManager::Instance()->SetExit(true);});

	UnitManager::Instance()->Run();

	std::cout << "qps:" << g_TotalCnt / (time / 1000) << std::endl;

	return true;
}
