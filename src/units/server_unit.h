// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_SERVER_UNIT_H
#define USRV_SERVER_UNIT_H

#include <thread>

#include "asio.hpp"

#include "components/loop.hpp"
#include "util/object_map.hpp"
#include "util/object_pool.hpp"
#include "util/spsc_queue.hpp"
#include "unit.h"

NAMESPACE_OPEN

enum class MSGTYPE
{
	MSGT_CONN = 0,
	MSGT_RECV,
	MSGT_DISC,
	MSGT_SEND,
};

static constexpr uint8_t IP_LEN_SIZE = sizeof(uint8_t);
static constexpr uint8_t IP_SIZE = 39;
static constexpr uint8_t PORT_SIZE = sizeof(PORT);
static constexpr uint8_t CONN_BUFFER_SIZE = IP_LEN_SIZE + IP_SIZE + PORT_SIZE;

using OnConnFunc = std::function<void(NETID, IP, PORT)>;
using OnRecvFunc = std::function<void(NETID, std::string&&)>;
using OnDiscFunc = std::function<void(NETID)>;

class Server;
class Peer;

class ServerUnit : public Unit, public std::enable_shared_from_this<ServerUnit>
{
public:
	ServerUnit(size_t thread_num, size_t pp_alloc_num, size_t ps_alloc_num, size_t spsc_size);
	virtual ~ServerUnit() = default;

	virtual bool Init() override final;
	virtual bool Start() override final;
	virtual void Stop() override final;
	virtual void Release() override final;

public:
	void Listen(PORT port);
	void Connect(const IP & ip, PORT port, OnConnFunc callback);
	void Disconnect(NETID net_id);
	void Recv(NETID net_id, MSGTYPE msg_type, std::string && msg);
	bool Send(NETID net_id, std::string && msg);
	void OnConn(OnConnFunc func);
	void OnRecv(OnRecvFunc func);
	void OnDisc(OnDiscFunc func);
	size_t PeersNum();

private:
	friend class Server;

	asio::io_context & _io_context;
	size_t _thread_num;
	size_t _pp_alloc_num;
	size_t _ps_alloc_num;
	size_t _spsc_size;
	std::vector<std::shared_ptr<asio::io_context>> _io_contexts;
	std::vector<asio::executor_work_guard<asio::io_context::executor_type>> _work_guards;
	std::vector<std::shared_ptr<Server>> _servers;
	std::vector<std::shared_ptr<std::thread>> _io_threads;

	OnConnFunc _on_conn;
	OnRecvFunc _on_recv;
	OnDiscFunc _on_disc;

	size_t _connect_idx = 0;
};

class ServerMsg
{
public:
	ServerMsg() = default;
	ServerMsg(PEERID pid, MSGTYPE msg_type, std::string && msg): pid(pid), msg_type(msg_type), msg(std::move(msg)) {}

	ServerMsg(ServerMsg&&) = default;
	ServerMsg& operator=(ServerMsg&&) = default;

	PEERID pid;
	MSGTYPE msg_type;
	std::string msg;
};

class Server : public std::enable_shared_from_this<Server>
{
public:
	Server(asio::io_context & io_context, size_t tid, size_t pp_alloc_num, size_t ps_alloc_num, size_t spsc_size, const std::shared_ptr<ServerUnit> & unit);
	virtual ~Server() = default;

	// 可以被主线程调用
	void Listen(PORT port);
	void Connect(IP ip, PORT port, OnConnFunc callback);
	void Disconnect(PEERID pid);
	void Recv(PEERID & pid, const MSGTYPE & msg_type, std::string && msg);
	bool Send(PEERID pid, std::string && msg);

private:
	// 在io_context中跑
	asio::awaitable<void> _Listen(PORT port);
	void _Recv(PEERID & pid, const MSGTYPE & msg_type, std::string && msg);
	asio::awaitable<void> _QueueRecv(PEERID pid, MSGTYPE msg_type, std::string msg);
	bool _UpdateRecv();
	asio::awaitable<void> _Send(PEERID pid, std::string msg);
	asio::awaitable<void> _QueueSend(PEERID pid, std::string msg);
	bool _UpdateSend();
	asio::awaitable<void> _Connect(IP ip, PORT port, OnConnFunc callback);
	void _Disconnect(PEERID pid);
	PEERID _AddPeer(asio::ip::tcp::socket && socket);
	void _DelPeer(PEERID pid);

private:
	friend class Peer;

	asio::io_context & _io_context;
	size_t _tid = -1;
	ObjectPool<Peer> _peer_pool;
	ObjectMap<Peer> _peers;
	std::shared_ptr<ServerUnit> _server_unit;

	char _conn_buffer[CONN_BUFFER_SIZE];

	std::shared_ptr<SpscQueue<ServerMsg>> _recv_queue;
	std::shared_ptr<SpscQueue<ServerMsg>> _send_queue;
	std::shared_ptr<asio::steady_timer> _recv_timer;
	std::shared_ptr<asio::steady_timer> _send_timer;
	std::shared_ptr<Loop> _recv_loop;
	std::shared_ptr<Loop> _send_loop;
};

// 在server的io_context运行
class Peer
{
public:
	Peer() = default;
	virtual ~Peer() = default;

public:
	void Start(PEERID pid, asio::ip::tcp::socket && socket, const std::shared_ptr<Server> & server);
	void Stop();
	void Release();
	asio::awaitable<void> Send(std::string && msg);
	IP Ip();
	PORT Port();

private:
	asio::awaitable<void> _Recv();
	void _CheckRelease();

private:
	PEERID _pid = INVALID_PEER_ID;
	std::shared_ptr<asio::ip::tcp::socket> _socket;
	std::shared_ptr<Server> _server;
	char _send_buffer[MESSAGE_HEAD_SIZE + MESSAGE_BODY_SIZE];
	char _recv_buffer[MESSAGE_BODY_SIZE];
	bool _wait_to_release;
	uint32_t _sending;
	bool _recving;
};

NAMESPACE_CLOSE

#endif // USRV_SERVER_UNIT_H
