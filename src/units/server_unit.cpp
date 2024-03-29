// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "server_unit.h"

#include "interfaces/logger_interface.h"
#include "util/time.h"
#include "unit_manager.h"

NAMESPACE_OPEN

// ServerUnit
ServerUnit::ServerUnit(size_t pp_alloc_num, size_t ps_alloc_num, size_t spsc_blk_num): _work_guard(asio::make_work_guard(_io_context)),
	_io_recv_timer(_io_context),
	_peer_pool(pp_alloc_num), _peers(ps_alloc_num), 
	_send_queue(spsc_blk_num), _recv_queue(spsc_blk_num)
{
}

bool ServerUnit::Init()
{
	_io_interval = _mgr->Interval();

	_io_thread = std::thread([self = shared_from_this()](){
		self->_IoInit();
	});

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
	auto busy = false;
	NETID net_id;
	MSGTYPE msg_type;
	uint16_t size;
	auto cnt = 0;
	while(_Recv(net_id, msg_type, _recv_buffer, size))
	{
		switch(msg_type)
		{
			case MSGTYPE::MSGT_CONN:
			{
				uint8_t ip_len = 0;
				memcpy(&ip_len, _recv_buffer, IP_LEN_SIZE);
				auto ip = std::string(_recv_buffer + IP_LEN_SIZE, ip_len);
				PORT port = 0;
				memcpy(&port, _recv_buffer + IP_LEN_SIZE + IP_SIZE, PORT_SIZE);
				_on_conn(net_id, std::move(ip), port);
			}
			break;
			case MSGTYPE::MSGT_RECV:
			{
				_on_recv(net_id, _recv_buffer, size);
			}
			break;
			case MSGTYPE::MSGT_DISC:
			{
				_on_disc(net_id);
			}
			break;
		}
		busy = true;
		if(++cnt >= OPF) break;
	}
	return busy;
}

void ServerUnit::Stop()
{
	_io_context.stop();
	if(_io_thread.joinable())
	{
		_io_thread.join();
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
	asio::co_spawn(_io_context, _IoListen(port), asio::detached);
}

NETID ServerUnit::Connect(const IP & ip, PORT port)
{
	try
	{
		std::promise<NETID> promise_net_id;
		auto future_net_id = promise_net_id.get_future();
		asio::co_spawn(_io_context, _IoConnect(ip, port, std::move(promise_net_id)), asio::detached);
		future_net_id.wait();
		return future_net_id.get();
	}
	catch(const std::system_error & e)
	{
		LOGGER_ERROR("ServerUnit::Connect error:{}", e.what());
		return INVALID_NET_ID;
	}
}

void ServerUnit::Disconnect(NETID net_id)
{
	asio::post(_io_context, [self = shared_from_this(), &net_id](){
		self->_IoDisconnect(net_id);
	});
}

bool ServerUnit::Send(NETID net_id, const char * data, uint16_t size)
{
	if(size > MESSAGE_BODY_SIZE)
	{
		LOGGER_ERROR("Send size too large size:{} max_size:{}", size, MESSAGE_BODY_SIZE);
		return false;
	}

	SpscQueue::Header header;
	header.size = size;
	header.data16 = net_id;
	header.data32 = 0;

	_send_queue.Push(data, header);

	asio::co_spawn(_io_context, _IoSend(), asio::detached);

	return true;
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
	std::promise<size_t> promise_num;
	auto future_num = promise_num.get_future();
	asio::post(_io_context, [self = shared_from_this(), &promise_num](){
		promise_num.set_value(self->_peers.Size());
	});
	return future_num.get();
}

bool ServerUnit::_Recv(NETID & net_id, MSGTYPE & msg_type, char * data, uint16_t & size)
{
	SpscQueue::Header header;

	if(!_recv_queue.TryPop(data, header))
	{
		return false;
	}

	size = header.size;
	net_id = header.data16;
	msg_type = (MSGTYPE)header.data32;

	return true;
}

void ServerUnit::_IoInit()
{
	_io_context.run();
}

asio::awaitable<void> ServerUnit::_IoSend()
{
	if(_sending)
	{
		co_return;
	}
	_sending = true;

	try
	{
		while(!_send_queue.Empty())
		{
			SpscQueue::Header header;

			if(!_send_queue.TryPop(_send_buffer, header))
			{
				continue;
			}

			if(!_peers.Find(header.data16))
			{
				continue;
			}

			co_await _peers[header.data16]->Send(_send_buffer, header.size);
		}
	}
	catch(const std::system_error & e)
	{
		LOGGER_ERROR("ServerUnit::_IoSend error:{}", e.what());
	}

	_sending = false;
}

asio::awaitable<void> ServerUnit::_IoListen(PORT port)
{
	try
	{
		asio::ip::tcp::acceptor acceptor(_io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), port));
		while(true)
		{
			auto socket = co_await acceptor.async_accept(asio::use_awaitable);
			co_await _IoAddPeer(std::move(socket));
		}
	}
	catch(const std::system_error & e)
	{
		LOGGER_ERROR("ServerUnit::_IoListen error:{}", e.what());
		Listen(port);
	}
}

asio::awaitable<void> ServerUnit::_IoConnect(const IP & ip, PORT port, std::promise<NETID> && promise_net_id)
{
	try
	{
		asio::ip::tcp::resolver resolver(_io_context);
		auto socket = asio::ip::tcp::socket(_io_context);
		auto endpoint = resolver.resolve(ip, std::to_string(port));
		co_await asio::async_connect(socket, endpoint, asio::use_awaitable);
		auto net_id = co_await _IoAddPeer(std::move(socket));
		promise_net_id.set_value(net_id);
	}
	catch(const std::system_error & e)
	{
		promise_net_id.set_exception(std::make_exception_ptr(e));
		LOGGER_ERROR("ServerUnit::_IoConnect error:{}", e.what());
	}
}

void ServerUnit::_IoDisconnect(NETID net_id)
{
	if(!_peers.Find(net_id))
	{
		return;
	}
	_peers[net_id]->Stop();
}

asio::awaitable<void> ServerUnit::_IoRecv(NETID net_id, const char * data, uint16_t size, MSGTYPE msg_type /* = MSGTYPE::MSGT_RECV */)
{
	SpscQueue::Header header;
	header.size = size;
	header.data16 = net_id;
	header.data32 = static_cast<uint32_t>(msg_type);
	while(!_recv_queue.TryPush(data, header))
	{
		_io_recv_timer.expires_after(ms_t(_io_interval));
		asio::error_code ec;
		co_await _io_recv_timer.async_wait(redirect_error(asio::use_awaitable, ec));
	}
	_mgr->LoopNotify();
}

asio::awaitable<NETID> ServerUnit::_IoAddPeer(asio::ip::tcp::socket && socket)
{
	auto net_id = _peers.Insert(std::move(_peer_pool.Get()));
	auto peer = _peers[net_id];
	peer->Start(net_id, std::move(socket), shared_from_this());

	auto ip = peer->Ip();
	auto ip_len = (uint8_t)ip.size();
	auto port = peer->Port();

	memcpy(_conn_buffer, &ip_len, IP_LEN_SIZE);
	memcpy(_conn_buffer + IP_LEN_SIZE, ip.c_str(), IP_SIZE);
	memcpy(_conn_buffer + IP_LEN_SIZE + IP_SIZE, &port, PORT_SIZE);

	co_await _IoRecv(net_id, _conn_buffer, CONN_BUFFER_SIZE, MSGTYPE::MSGT_CONN);

	co_return net_id;
}

asio::awaitable<void> ServerUnit::_IoDelPeer(NETID net_id)
{
	_peers[net_id]->Release();
	_peer_pool.Put(std::move(_peers[net_id]));
	_peers.Erase(net_id);

	co_await _IoRecv(net_id, "", 0, MSGTYPE::MSGT_DISC);
}

// Peer
void Peer::Start(NETID net_id, asio::ip::tcp::socket && socket, const std::shared_ptr<ServerUnit> & server)
{
	_net_id = net_id;
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
	_net_id = INVALID_NET_ID;
	_socket = nullptr;
	_server = nullptr;
}

asio::awaitable<void> Peer::Send(const char * data, uint16_t size)
{
	++_sending;
	try
	{
		auto nsize = htons(size);
		memcpy(_send_buffer, &nsize, MESSAGE_HEAD_SIZE);
		memcpy(_send_buffer + MESSAGE_HEAD_SIZE, data, size);
		co_await asio::async_write(*_socket, asio::buffer(_send_buffer, MESSAGE_HEAD_SIZE + size), asio::use_awaitable);
	}
	catch (const std::system_error & e)
	{
		if(e.code() != asio::error::eof && e.code() != asio::error::connection_reset)
		{
			LOGGER_ERROR("Peer::Send error:{}", e.what());
		}
		_server->_IoDisconnect(_net_id);
	}
	--_sending;
	co_await _CheckRelease();
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

			co_await _server->_IoRecv(_net_id, _recv_buffer, body_size);
		}
	}
	catch (const std::system_error & e)
	{
		if(e.code() != asio::error::eof && e.code() != asio::error::connection_reset)
		{
			LOGGER_ERROR("Peer::_Recv error:{}", e.what());
		}
		_server->_IoDisconnect(_net_id);
	}
	_recving = false;
	co_await _CheckRelease();
}

asio::awaitable<void> Peer::_CheckRelease()
{
	if(!_wait_to_release || _sending > 0 || _recving)
	{
		co_return;
	}
	co_await _server->_IoDelPeer(_net_id);
}

NAMESPACE_CLOSE
