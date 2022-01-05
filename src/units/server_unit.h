// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_SERVER_UNIT_H
#define USRV_SERVER_UNIT_H

#include <map>
#include <thread>

#include "asio.hpp"

#include "unit.h"
#include "util/common.h"
#include "util/object_pool.h"
#include "util/spsc_queue.hpp"

NAMESPACE_OPEN

class Peer;

class ServerUnit : public Unit, public std::enable_shared_from_this<ServerUnit>
{
public:
	ServerUnit(size_t peer_pool_num, size_t spsc_blk_num);
	virtual ~ServerUnit() = default;

	virtual bool Start() override final;
	virtual void Update(intvl_t interval) override final;
	virtual void Stop() override final;

public:
	void Listen(PORT port);
	void Connect(IP ip, PORT port);
	void Disconnect(NETID net_id);
	bool Send(NETID net_id, const char * data, uint16_t size);
	bool Recv(NETID * net_id, char * data, uint16_t * size);

private:
	void _IoStart();
	asio::awaitable<void> _IoUpdate();
	asio::awaitable<void> _IoListen(PORT port);
	asio::awaitable<void> _IoConnect(IP ip, PORT port);
	void _IoAddPeer(asio::ip::tcp::socket && socket);
	void _IoDelPeer(const NETID & net_id);
	NETID _IoGetNetID() { return ++_net_id; }

private:
	std::thread _io_thread;
	asio::io_context _io_context;
	asio::steady_timer _timer;
	intvl_t _io_interval;

	NETID _net_id = 0;
	ObjectPool<Peer> _peer_pool;
	std::map<NETID, std::shared_ptr<Peer>> _peers;

	friend class Peer;
	SpscQueue _send_queue;
	SpscQueue _recv_queue;
	char _send_buff[MESSAGE_BODY_SIZE];
};

class Peer : public std::enable_shared_from_this<Peer> 
{
public:
	Peer() = default;
	virtual ~Peer() = default;

public:
	void Start(const NETID & net_id, asio::ip::tcp::socket && socket, std::shared_ptr<ServerUnit> server);
	void Stop();
	void Send(const char * data, size_t size);

private:
	asio::awaitable<void> _Send(const char * data, size_t size);
	asio::awaitable<void> _Recv();

private:
	NETID _net_id = INVALID_NET_ID;
	std::shared_ptr<asio::ip::tcp::socket> _socket;
	std::shared_ptr<ServerUnit> _server;
	char _recv_buff[MESSAGE_BODY_SIZE];
};

NAMESPACE_CLOSE

#endif // USRV_SERVER_UNIT_H
