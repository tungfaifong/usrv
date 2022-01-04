// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_SERVER_UNIT_H
#define USRV_SERVER_UNIT_H

#include <map>
#include <thread>

#include "asio.hpp"

#include "unit.h"
#include "util/common.h"
#include "util/spsc_queue.hpp"

namespace usrv
{

class Peer;

class ServerUnit : public Unit
{
public:
	using MsgQueue = SpscQueue<SPSC_QUEUE_BLOCK_NUM>;

	ServerUnit();
	virtual ~ServerUnit();

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
	asio::awaitable<void> _IoListen(asio::ip::tcp::acceptor & acceptor);
	asio::awaitable<void> _IoConnect(asio::ip::tcp::resolver & resolver, IP ip, PORT port);
	void _IoAddPeer(asio::ip::tcp::socket socket);
	void _IoDelPeer(const NETID & net_id);
	NETID _IoGetNetID() { return ++_net_id; }

private:
	std::thread _io_thread;
	asio::io_context _io_context;
	asio::steady_timer _timer;

	NETID _net_id = 0;
	std::map<NETID, std::shared_ptr<Peer>> _peers;

	friend class Peer;
	MsgQueue _send_queue;
	MsgQueue _recv_queue;
	char _send_buff[MESSAGE_BODY_SIZE];
};

class Peer : public std::enable_shared_from_this<Peer> 
{
public:
	Peer(const NETID & net_id, asio::ip::tcp::socket socket, ServerUnit & server);
	virtual ~Peer();

public:
	void Start();
	void Stop();
	void Send(const char * data, size_t size);

private:
	asio::awaitable<void> _Send(const char * data, size_t size);
	asio::awaitable<void> _Recv();

private:
	NETID _net_id = INVALID_NET_ID;
	asio::ip::tcp::socket _socket;
	ServerUnit & _server;
	char _recv_buff[MESSAGE_BODY_SIZE];
};

}

#endif // USRV_SERVER_UNIT_H
