// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "server_unit.h"

#include "unit_manager.h"

NAMESPACE_OPEN

// ServerUnit
ServerUnit::ServerUnit(size_t peer_pool_num, size_t spsc_blk_num): _timer(_io_context), _peer_pool(peer_pool_num), _send_queue(spsc_blk_num), _recv_queue(spsc_blk_num)
{
}

bool ServerUnit::Start()
{
	_io_interval = _mgr->Interval();

	_io_thread = std::thread([self = shared_from_this()](){
		self->_IoStart();
	});

	return true;
}

void ServerUnit::Update(intvl_t interval)
{
}

void ServerUnit::Stop()
{
	_io_context.stop();
	_io_thread.join();
}

void ServerUnit::Listen(PORT port)
{
	asio::co_spawn(_io_context, _IoListen(port), asio::detached);
}

void ServerUnit::Connect(IP ip, PORT port)
{
	asio::co_spawn(_io_context, _IoConnect(ip, port), asio::detached);
}

void ServerUnit::Disconnect(NETID net_id)
{
	asio::post(_io_context, [self = shared_from_this(), &net_id](){
		auto peer = self->_peers.find(net_id);
		if(peer == self->_peers.end())
		{
			return;
		}
		peer->second->Stop();
	});
}

bool ServerUnit::Send(NETID net_id, const char * data, uint16_t size)
{
	SpscQueue::Header header;
	header.size = size;
	header.data16 = net_id;
	header.data32 = 0;

	if(!_send_queue.TryPush(data, header))
	{
		return false;
	}

	return true;
}

bool ServerUnit::Recv(NETID * net_id, char * data, uint16_t * size)
{
	SpscQueue::Header header;

	if(!_recv_queue.TryPop(data, &header))
	{
		return false;
	}

	*size = header.size;
	*net_id = header.data16;

	return true;
}

void ServerUnit::_IoStart()
{
	asio::co_spawn(_io_context, _IoUpdate(), asio::detached);

	_io_context.run();
}

asio::awaitable<void> ServerUnit::_IoUpdate()
{
	while(true)
	{
		while(!_send_queue.Empty())
		{
			SpscQueue::Header header;

			if(!_send_queue.TryPop(_send_buff, &header))
			{
				continue;
			}

			auto peer = _peers.find(header.data16);
			if(peer == _peers.end())
			{
				continue;
			}

			peer->second->Send(_send_buff, header.size);
		}

		_timer.expires_after(std::chrono::milliseconds(_io_interval));
		asio::error_code ec;
		co_await _timer.async_wait(redirect_error(asio::use_awaitable, ec));
	}
}

asio::awaitable<void> ServerUnit::_IoListen(PORT port)
{
	asio::ip::tcp::acceptor acceptor(_io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), port));
	while(true)
	{
		auto socket = co_await acceptor.async_accept(asio::use_awaitable);
		_IoAddPeer(std::move(socket));
	}
}

asio::awaitable<void> ServerUnit::_IoConnect(IP ip, PORT port)
{
	asio::ip::tcp::resolver resolver(_io_context);
	auto socket = asio::ip::tcp::socket(_io_context);
	auto endpoint = resolver.resolve(ip, std::to_string(port));
	co_await asio::async_connect(socket, endpoint, asio::use_awaitable);
	_IoAddPeer(std::move(socket));
}

void ServerUnit::_IoAddPeer(asio::ip::tcp::socket && socket)
{
	auto net_id = _IoGetNetID();
	_peers[net_id] = std::move(_peer_pool.Get());
	_peers[net_id]->Start(net_id, std::move(socket), shared_from_this());
}

void ServerUnit::_IoDelPeer(const NETID & net_id)
{
	auto peer = _peers.find(net_id);
	if(peer == _peers.end())
	{
		return;
	}
	_peer_pool.Put(std::move(peer->second));
	_peers.erase(net_id);
}

// Peer
void Peer::Start(const NETID & net_id, asio::ip::tcp::socket && socket, std::shared_ptr<ServerUnit> server)
{
	_net_id = net_id;
	_socket = std::make_shared<asio::ip::tcp::socket>(std::move(socket));
	_server = server;
	asio::co_spawn(_socket->get_executor(), _Recv(), asio::detached);
}

void Peer::Stop()
{
	_socket->close();
	_server->_IoDelPeer(_net_id);
	_net_id = INVALID_NET_ID;
	_socket = nullptr;
	_server = nullptr;
}

void Peer::Send(const char * data, size_t size)
{
	asio::co_spawn(_socket->get_executor(), _Send(data, size), asio::detached);
}

asio::awaitable<void> Peer::_Send(const char * data, size_t size)
{
	try
	{
		co_await asio::async_write(*_socket, asio::buffer(data, size), asio::use_awaitable);
	}
	catch (std::exception& e)
	{
		Stop();
	}
}

asio::awaitable<void> Peer::_Recv()
{
	try
	{
		while(true)
		{
			uint16_t body_size = 0;
			co_await asio::async_read(*_socket, asio::buffer(&body_size, MESSAGE_HEAD_SIZE), asio::use_awaitable);
			co_await asio::async_read(*_socket, asio::buffer(_recv_buff, body_size), asio::use_awaitable);

			SpscQueue::Header header;
			header.size = body_size;
			header.data16 = _net_id;
			header.data32 = 0;

			if(!_server->_recv_queue.TryPush(_recv_buff, header))
			{

			}
		}
	}
	catch (std::exception& e)
	{
		Stop();
	}
}

NAMESPACE_CLOSE
