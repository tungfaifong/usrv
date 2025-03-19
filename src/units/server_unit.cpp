// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "server_unit.h"

#include "interfaces/logger_interface.h"
#include "util/time.h"
#include "unit_manager.h"

NAMESPACE_OPEN

// ServerUnit
ServerUnit::ServerUnit(size_t thread_num, size_t pp_alloc_num, size_t ps_alloc_num, size_t spsc_blk_num): _thread_num(thread_num),
	_pp_alloc_num(pp_alloc_num),
	_ps_alloc_num(ps_alloc_num),
	_spsc_blk_num(spsc_blk_num)
{
}

bool ServerUnit::Init()
{
	if(_thread_num == 0)
	{
		_servers.push_back(std::move(std::make_shared<Server>(UnitManager::Instance()->IOContext(), 0, _pp_alloc_num, _ps_alloc_num, _spsc_blk_num, shared_from_this())));
	}
	else
	{
		for(size_t i = 0; i < _thread_num; ++i)
		{
			_io_contexts.emplace_back(new asio::io_context());
			_work_guards.emplace_back(asio::make_work_guard(*_io_contexts[i]));
			_servers.emplace_back(std::move(std::make_shared<Server>(*_io_contexts[i], i, _pp_alloc_num, _ps_alloc_num, _spsc_blk_num, shared_from_this())));
			_io_threads.emplace_back(new std::thread([this, &i](){
				_io_contexts[i]->run();
			}));
		}
	}

	return true;
}

bool ServerUnit::Start()
{
	if(!_on_conn)
	{
		LOGGER_ERROR("ServerUnit::Start error: no _on_conn, please call OnConn(OnConnFunc func)");
		return false;
	}

	if(!_on_recv)
	{
		LOGGER_ERROR("ServerUnit::Start error: no _on_recv, please call OnRecv(OnRecvFunc func)");
		return false;
	}
	
	if(!_on_disc)
	{
		LOGGER_ERROR("ServerUnit::Start error: no _on_disc, please call OnDisc(OnDiscFunc func)");
		return false;
	}

	return true;
}

bool ServerUnit::Update(intvl_t interval)
{
	// auto busy = false;
	// NETID net_id;
	// MSGTYPE msg_type;
	// uint16_t size;
	// auto cnt = 0;
	// while(_Recv(net_id, msg_type, _recv_buffer, size))
	// {
	// 	switch(msg_type)
	// 	{
	// 		case MSGTYPE::MSGT_CONN:
	// 		{
	// 			uint8_t ip_len = 0;
	// 			memcpy(&ip_len, _recv_buffer, IP_LEN_SIZE);
	// 			auto ip = std::string(_recv_buffer + IP_LEN_SIZE, ip_len);
	// 			PORT port = 0;
	// 			memcpy(&port, _recv_buffer + IP_LEN_SIZE + IP_SIZE, PORT_SIZE);
	// 			_on_conn(net_id, std::move(ip), port);
	// 		}
	// 		break;
	// 		case MSGTYPE::MSGT_RECV:
	// 		{
	// 			_on_recv(net_id, _recv_buffer, size);
	// 		}
	// 		break;
	// 		case MSGTYPE::MSGT_DISC:
	// 		{
	// 			_on_disc(net_id);
	// 		}
	// 		break;
	// 	}
	// 	busy = true;
	// 	if(++cnt >= OPF) break;
	// }
	// return busy;
	return true;
}

void ServerUnit::Stop()
{
	for(size_t i = 0; i < _io_threads.size(); ++i)
	{
		_io_contexts[i]->stop();
		if(_io_threads[i]->joinable())
		{
			_io_threads[i]->join();
		}
	}
}

void ServerUnit::Release()
{
	_on_conn = nullptr;
	_on_recv = nullptr;
	_on_disc = nullptr;
	Unit::Release();
}

void ServerUnit::Listen(PORT port)
{
	for(size_t i = 0; i < _servers.size(); ++i)
	{
		_servers[i]->Listen(port);
	}
}

void ServerUnit::Connect(const IP & ip, PORT port, OnConnFunc callback)
{
	_servers[0]->Connect(ip, port, callback);
}

void ServerUnit::Disconnect(NETID net_id)
{
	_servers[net_id.tid]->Disconnect(net_id.pid);
}

bool ServerUnit::Send(NETID net_id, std::string && msg)
{
	return _servers[net_id.tid]->Send(net_id.pid, msg);
}

void ServerUnit::OnConn(OnConnFunc func)
{
	_on_conn = func;
}

void ServerUnit::OnRecv(OnRecvFunc func)
{
	_on_recv = func;
}

void ServerUnit::OnDisc(OnDiscFunc func)
{
	_on_disc = func;
}

size_t ServerUnit::PeersNum()
{
	// std::promise<size_t> promise_num;
	// auto future_num = promise_num.get_future();
	// asio::post(_io_context, [self = shared_from_this(), &promise_num](){
	// 	promise_num.set_value(self->_peers.Size());
	// });
	// return future_num.get();
	return 0;
}

bool ServerUnit::_Recv(NETID & net_id, const MSGTYPE & msg_type, std::string && msg)
{
	switch(msg_type)
	{
		case MSGTYPE::MSGT_CONN:
		{
			uint8_t ip_len = 0;
			memcpy(&ip_len, msg.data(), IP_LEN_SIZE);
			auto ip = std::string(msg.data() + IP_LEN_SIZE, ip_len);
			PORT port = 0;
			memcpy(&port, msg.data() + IP_LEN_SIZE + IP_SIZE, PORT_SIZE);
			_on_conn(net_id, std::move(ip), port);
		}
		break;
		case MSGTYPE::MSGT_RECV:
		{
			_on_recv(net_id, std::move(msg));
		}
		break;
		case MSGTYPE::MSGT_DISC:
		{
			_on_disc(net_id);
		}
		break;
	}

	return true;
}

// Server
Server::Server(asio::io_context & io_context, size_t tid, size_t pp_alloc_num, size_t ps_alloc_num, size_t spsc_blk_num, const std::shared_ptr<ServerUnit> & unit): _io_context(io_context),
	_tid(tid),
	_peer_pool(pp_alloc_num), _peers(ps_alloc_num),
	_server_unit(unit)
{
}

void Server::Listen(PORT port)
{
	asio::co_spawn(_io_context, _Listen(port), asio::detached);
}

void Server::Connect(IP ip, PORT port, OnConnFunc callback)
{
	try
	{
		asio::co_spawn(_io_context, _Connect(ip, port, callback), asio::detached);
	}
	catch(const std::system_error & e)
	{
		LOGGER_ERROR("ServerUnit::Connect error:{}", e.what());
	}
}

void Server::Disconnect(PEERID pid)
{
	asio::post(_io_context, [self = shared_from_this(), &pid](){
		self->_Disconnect(pid);
	});
}

bool Server::Send(PEERID pid, std::string msg)
{
	if(msg.size() > MESSAGE_BODY_SIZE)
	{
		LOGGER_ERROR("Send size too large size:{} max_size:{}", msg.size(), MESSAGE_BODY_SIZE);
		return false;
	}

	asio::co_spawn(_io_context, _Send(pid, std::move(msg)), asio::detached);

	return true;
}

asio::awaitable<void> Server::_Listen(PORT port)
{
	try
	{
		asio::ip::tcp::acceptor acceptor(_io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), port));
		while(true)
		{
			auto socket = co_await acceptor.async_accept(asio::use_awaitable);
			_AddPeer(std::move(socket));
		}
	}
	catch(const std::system_error & e)
	{
		LOGGER_ERROR("ServerUnit::_Listen error:{}", e.what());
	}
}

bool Server::_Recv(PEERID & pid, const MSGTYPE & msg_type, std::string && msg)
{
	NETID nid;
	nid.tid = _tid;
	nid.pid = pid;
	return _server_unit->_Recv(nid, msg_type, std::move(msg));
}

asio::awaitable<void> Server::_Send(PEERID pid, std::string && msg)
{
	try
	{
		if(!_peers.Find(pid))
		{
			co_return;
		}
		co_await _peers[pid]->Send(std::move(msg));
	}
	catch(const std::system_error & e)
	{
		LOGGER_ERROR("ServerUnit::_Send error:{}", e.what());
	}
}

asio::awaitable<void> Server::_Connect(IP ip, PORT port, OnConnFunc callback)
{
	try
	{
		asio::ip::tcp::resolver resolver(_io_context);
		auto socket = asio::ip::tcp::socket(_io_context);
		auto endpoint = resolver.resolve(ip, std::to_string(port));
		co_await asio::async_connect(socket, endpoint, asio::use_awaitable);
		auto pid = _AddPeer(std::move(socket));
		NETID nid;
		nid.tid = _tid;
		nid.pid = pid;
		callback(nid, ip, port);
	}
	catch(const std::system_error & e)
	{
		LOGGER_ERROR("ServerUnit::_Connect error:{}", e.what());
	}
}

void Server::_Disconnect(PEERID pid)
{
	if(!_peers.Find(pid))
	{
		return;
	}
	_peers[pid]->Stop();
}

PEERID Server::_AddPeer(asio::ip::tcp::socket && socket)
{
	auto pid = _peers.Insert(std::move(_peer_pool.Get()));
	auto peer = _peers[pid];
	peer->Start(pid, std::move(socket), shared_from_this());

	auto ip = peer->Ip();
	auto ip_len = (uint8_t)ip.size();
	auto port = peer->Port();

	memcpy(_conn_buffer, &ip_len, IP_LEN_SIZE);
	memcpy(_conn_buffer + IP_LEN_SIZE, ip.data(), IP_SIZE);
	memcpy(_conn_buffer + IP_LEN_SIZE + IP_SIZE, &port, PORT_SIZE);

	_Recv(pid, MSGTYPE::MSGT_CONN, std::move(std::string(_conn_buffer, CONN_BUFFER_SIZE)));

	return pid;
}

void Server::_DelPeer(PEERID pid)
{
	_peers[pid]->Release();
	_peer_pool.Put(std::move(_peers[pid]));
	_peers.Erase(pid);

	_Recv(pid, MSGTYPE::MSGT_DISC, std::move(std::string("")));
}

// Peer
void Peer::Start(PEERID pid, asio::ip::tcp::socket && socket, const std::shared_ptr<Server> & server)
{
	_pid = pid;
	_socket = std::make_shared<asio::ip::tcp::socket>(std::move(socket));
	_server = server;
	_wait_to_release = false;
	_sending = 0;
	_recving = false;
	asio::co_spawn(_socket->get_executor(), _Recv(), asio::detached);
}

void Peer::Stop()
{
	try
	{
		if(_socket->is_open())
		{
			std::error_code ec;
			_socket->shutdown(asio::ip::tcp::socket::shutdown_both, ec);
			if(ec && ec != asio::error::not_connected)
			{
				LOGGER_ERROR("Peer::Stop shutdown error:{}", ec.message());
			}
			_socket->close(ec);
			_wait_to_release = true;
		}
	}
	catch(const std::system_error& e)
	{
		LOGGER_ERROR("Peer::Stop error:{}", e.what());
	}
}

void Peer::Release()
{
	_pid = INVALID_PEER_ID;
	_socket = nullptr;
	_server = nullptr;
}

asio::awaitable<void> Peer::Send(std::string && msg)
{
	++_sending;
	try
	{
		auto nsize = htons(msg.size());
		memcpy(_send_buffer, &nsize, MESSAGE_HEAD_SIZE);
		memcpy(_send_buffer + MESSAGE_HEAD_SIZE, msg.data(), msg.size());
		co_await asio::async_write(*_socket, asio::buffer(_send_buffer, MESSAGE_HEAD_SIZE + msg.size()), asio::use_awaitable);
	}
	catch (const std::system_error & e)
	{
		if(e.code() != asio::error::eof && e.code() != asio::error::connection_reset)
		{
			LOGGER_ERROR("Peer::Send error:{}", e.what());
		}
		_server->_Disconnect(_pid);
	}
	--_sending;
	_CheckRelease();
}

IP Peer::Ip()
{
	return _socket->remote_endpoint().address().to_string();
}

PORT Peer::Port()
{
	return _socket->remote_endpoint().port();
}

asio::awaitable<void> Peer::_Recv()
{
	_recving = true;
	try
	{
		while(_socket)
		{
			uint16_t body_size = 0;

			co_await asio::async_read(*_socket, asio::buffer(&body_size, MESSAGE_HEAD_SIZE), asio::use_awaitable);
			if(!_socket->is_open()) break;
			body_size = ntohs(body_size);
			co_await asio::async_read(*_socket, asio::buffer(_recv_buffer, body_size), asio::use_awaitable);

			std::string msg(_recv_buffer, body_size);
			_server->_Recv(_pid, MSGTYPE::MSGT_RECV, std::move(msg));
		}
	}
	catch (const std::system_error & e)
	{
		if(e.code() != asio::error::eof && e.code() != asio::error::connection_reset)
		{
			LOGGER_ERROR("Peer::_Recv error:{}", e.what());
		}
		_server->_Disconnect(_pid);
	}
	_recving = false;
	_CheckRelease();
}

void Peer::_CheckRelease()
{
	if(!_wait_to_release || _sending > 0 || _recving)
	{
		return;
	}
	_server->_DelPeer(_pid);
}

NAMESPACE_CLOSE
