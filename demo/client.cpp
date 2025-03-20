// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "interfaces/logger_interface.h"
#include "interfaces/server_interface.h"
#include "interfaces/timer_interface.h"
#include "unit_manager.h"
#include "units/server_unit.h"

using namespace usrv;

#include <random>

std::string generate_random_string(size_t length = 12, 
	const std::string& charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
							   "abcdefghijklmnopqrstuvwxyz"
							   "0123456789") {
	// 初始化随机数生成器
	std::random_device rd;
	std::mt19937 generator(rd());

	// 创建均匀分布，范围是字符集的有效索引
	std::uniform_int_distribution<size_t> distribution(0, charset.size() - 1);

	std::string result;
	result.reserve(length);

	for (size_t i = 0; i < length; ++i) {
	result += charset[distribution(generator)];
	}

	return result;
}

class Stat
{
public:
	Stat() {}
	~Stat() = default;

	static uint32_t client_num;
	static uint32_t req_num;
	static intvl_t min_delay;
	static intvl_t max_delay;
	static intvl_t total_delay;
	static sys_clock_t start_clock;
	static sys_clock_t end_clock;

	static uint32_t connected_num;

};

uint32_t Stat::client_num = 0;
uint32_t Stat::req_num = 0;
intvl_t Stat::min_delay = UINT64_MAX;
intvl_t Stat::max_delay = 0;
intvl_t Stat::total_delay = 0;
sys_clock_t Stat::start_clock = SysNow();
sys_clock_t Stat::end_clock = SysNow();
uint32_t Stat::connected_num = 0;

class Client;

std::unordered_map<NETID, std::shared_ptr<Client>, NETIDHash> g_Clients;

class Client : public Unit, public std::enable_shared_from_this<Client>
{
public:
	Client() {}
	~Client() = default;

	virtual bool Start();
	void OnRecv(NETID net_id, std::string && msg);
	void Send();

	NETID _server_net_id = INVALID_NET_ID;
	sys_clock_t _send_clock;
	sys_clock_t _recv_clock;
};

bool Client::Start()
{
	server::Connect("127.0.0.1", 6666, [this](NETID net_id, IP ip, PORT port){
		_server_net_id = net_id;
		if(_server_net_id == INVALID_NET_ID)
		{
			LOGGER_ERROR("client connect failed.");
			return;
		}
		g_Clients[_server_net_id] = shared_from_this();
		++Stat::connected_num;
	});
	return true;
}

void Client::OnRecv(NETID net_id, std::string && msg)
{
	// LOGGER_INFO("RECV msg {}", msg);
	_recv_clock = SysNow();
	intvl_t delay = (_recv_clock - _send_clock).count();
	Stat::total_delay += delay;
	if(delay < Stat::min_delay)
	{
		Stat::min_delay = delay;
	}
	if(delay > Stat::max_delay)
	{
		Stat::max_delay = delay;
	}
	--Stat::req_num;
	if(Stat::req_num > 0)
	{
		Send();
	}
	else
	{
		UnitManager::Instance()->SetExit(true);
	}
}

void Client::Send()
{
	std::string msg = "123"; //generate_random_string(rand() % 100);
	// LOGGER_INFO("SEND msg {}", msg);
	server::Send(_server_net_id, std::move(msg));
	_send_clock = SysNow();
}

class Mgr : public Unit, public std::enable_shared_from_this<Mgr>
{
public:
	virtual bool Start() {return true;}
	virtual bool Update(intvl_t interval);

	bool start = false;
};

bool Mgr::Update(intvl_t interval)
{
	if(Stat::connected_num == Stat::client_num && !start)
	{
		Stat::start_clock = SysNow();
		for(uint32_t i = 0; i < Stat::client_num; ++i)
		{
			auto client_key = "CLIENT#"+std::to_string(i+1);
			auto client = std::dynamic_pointer_cast<Client>(UnitManager::Instance()->Get(client_key));
			client->Send();
		}
		start = true;
	}
	
	return false;
}

bool run_client(uint32_t client_num, uint32_t req_num, size_t thread_num)
{
	SignalInit();

	Stat::client_num = client_num;
	Stat::req_num = req_num;

	UnitManager::Instance()->Init(10);
	UnitManager::Instance()->Register("LOGGER", std::move(std::make_shared<LoggerUnit>(LoggerUnit::LEVEL::TRACE, "/logs/client.log", 1 Ki)));
	UnitManager::Instance()->Register("TIMER", std::move(std::make_shared<TimerUnit>(1 Ki, 1 Ki)));
	UnitManager::Instance()->Register("SERVER", std::move(std::make_shared<ServerUnit>(thread_num, 1 Ki, 1 Ki, 4 Ki)));

	for(uint32_t i = 0; i < client_num; ++i)
	{
		auto client_key = "CLIENT#"+std::to_string(i+1);
		if(!UnitManager::Instance()->Register(client_key.c_str(), std::move(std::make_shared<Client>())))
		{
			LOGGER_ERROR("key:{} error", client_key);
		}
	}

	UnitManager::Instance()->Register("MGR", std::move(std::make_shared<Mgr>()));

	auto server = std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"));
	server->OnConn([](NETID net_id, IP ip, PORT port){
		LOGGER_INFO("conn: net_id:{} ip:{} port:{}", net_id.pid, ip, port);
	});
	server->OnRecv([](NETID net_id, std::string && msg) {
		auto client = g_Clients.find(net_id);
		if(client != g_Clients.end())
		{
			client->second->OnRecv(net_id, std::move(msg));
		}
	});
	server->OnDisc([](NETID net_id){
		LOGGER_INFO("disc: net_id:{}", net_id.pid);
	});

	UnitManager::Instance()->Run();
	Stat::end_clock = SysNow();

	std::cout << "rps:" << req_num / (static_cast<double>((Stat::end_clock - Stat::start_clock).count()) / 1000000000) << std::endl;
	std::cout << "avg delay:" << (static_cast<double>(Stat::total_delay) / 1000000) / req_num << std::endl;
	std::cout << "min delay:" << (static_cast<double>(Stat::min_delay) / 1000000) << std::endl;
	std::cout << "max delay:" << (static_cast<double>(Stat::max_delay) / 1000000) << std::endl;

	return true;
}
