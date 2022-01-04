// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "server_unit.h"

namespace usrv
{

// ServerUnit
ServerUnit::ServerUnit(): _timer(_io_context)
{
}

ServerUnit::~ServerUnit()
{
}

bool ServerUnit::Start()
{
	_io_thread = std::thread([this](){
		_IoStart();
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
	asio::ip::tcp::acceptor acceptor(_io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), port));
	asio::co_spawn(_io_context, _IoListen(acceptor), asio::detached);
}

void ServerUnit::Connect(IP ip, PORT port)
{
	asio::ip::tcp::resolver resolver(_io_context);
	asio::co_spawn(_io_context, _IoConnect(resolver, ip, port), asio::detached);
}

void ServerUnit::Disconnect(NETID net_id)
{
	asio::post(_io_context, [this, &net_id](){
		auto peer = _peers.find(net_id);
		if(peer == _peers.end())
		{
			return;
		}
		peer->second->Stop();
	});
}

bool ServerUnit::Send(NETID net_id, const char * data, uint16_t size)
{
	MsgQueue::Header header;
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
	MsgQueue::Header header;

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
			MsgQueue::Header header;

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

		_timer.expires_after(std::chrono::milliseconds(10));
		asio::error_code ec;
		co_await _timer.async_wait(redirect_error(asio::use_awaitable, ec));
	}
}

asio::awaitable<void> ServerUnit::_IoListen(asio::ip::tcp::acceptor & acceptor)
{
	while(true)
	{
		auto socket = co_await acceptor.async_accept(asio::use_awaitable);
		_IoAddPeer(std::move(socket));
	}
}

asio::awaitable<void> ServerUnit::_IoConnect(asio::ip::tcp::resolver & resolver, IP ip, PORT port)
{
	auto socket = asio::ip::tcp::socket(_io_context);
	auto endpoint = resolver.resolve(ip, std::to_string(port));
	co_await asio::async_connect(socket, endpoint, asio::use_awaitable);
	_IoAddPeer(std::move(socket));
}

void ServerUnit::_IoAddPeer(asio::ip::tcp::socket socket)
{
	auto net_id = _IoGetNetID();
	auto peer = std::make_shared<Peer>(net_id, std::move(socket), *this);
	_peers[net_id] = std::move(peer);
	_peers[net_id]->Start();
}

void ServerUnit::_IoDelPeer(const NETID & net_id)
{
	_peers.erase(net_id);
}

// Peer
Peer::Peer(const NETID & net_id, asio::ip::tcp::socket socket, ServerUnit & server):_net_id(net_id), _socket(std::move(socket)), _server(server)
{
}

Peer::~Peer()
{
}

void Peer::Start()
{
	asio::co_spawn(_socket.get_executor(), _Recv(), asio::detached);
}

void Peer::Stop()
{
	_socket.close();
	_server._IoDelPeer(_net_id);
}

void Peer::Send(const char * data, size_t size)
{
	asio::co_spawn(_socket.get_executor(), _Send(data, size), asio::detached);
}

asio::awaitable<void> Peer::_Send(const char * data, size_t size)
{
	try
	{
		co_await asio::async_write(_socket, asio::buffer(data, size), asio::use_awaitable);
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
			co_await asio::async_read(_socket, asio::buffer(&body_size, MESSAGE_HEAD_SIZE), asio::use_awaitable);
			co_await asio::async_read(_socket, asio::buffer(_recv_buff, body_size), asio::use_awaitable);

			ServerUnit::MsgQueue::Header header;
			header.size = body_size;
			header.data16 = _net_id;
			header.data32 = 0;

			if(!_server._recv_queue.TryPush(_recv_buff, header))
			{

			}
		}
	}
	catch (std::exception& e)
	{
		Stop();
	}
}

}
