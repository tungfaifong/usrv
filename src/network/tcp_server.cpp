// Copyright (c) 2019-2020 TungFai Fong 

#include "tcp_server.h"
#include "tcp_server_impl.h"

namespace usrv
{
    //TcpServerImpl
    TcpServer::Impl::Impl(asio::io_context & io_context) : io_context_(io_context),
        acceptor_(nullptr),
        resolver_(nullptr),
        peer_pool_(),
        connect_peers_(nullptr),
        msgs_(nullptr),
        disconnect_peers_(nullptr),
        on_connect_(nullptr),
        on_recv_(nullptr),
        on_disconnect_(nullptr)
    {
        InitPool();
    }

    TcpServer::Impl::~Impl()
    {

    }

    void TcpServer::Impl::Update(clock_t interval)
    {
        if (connect_peers_ != nullptr && on_connect_ != nullptr)
        {
            static const auto connect_peers_each_func = [this](const ConnectPeer & peer)
            {
                on_connect_(peer.net_id_, peer.ip_, peer.port_);
            };

            connect_peers_->ForEach(connect_peers_each_func);
        }

        if (msgs_ != nullptr && on_recv_ != nullptr)
        {
            static const auto msgs_each_func = [this](const TcpMessage & msg)
            {
                on_recv_(msg.GetNetID(), msg.Data(), msg.Size());
            };

            msgs_->ForEach(msgs_each_func);
        }

        if (disconnect_peers_ != nullptr && on_disconnect_ != nullptr)
        {
            static const auto disconnect_peers_each_func = [this](const NetID & peer)
            {
                on_disconnect_(peer);
            };

            disconnect_peers_->ForEach(disconnect_peers_each_func);
        }
    }

    void TcpServer::Impl::InitPool()
    {
        static const auto destructor = [this](TcpPeer * peer)
        {
            peer->Release();
        };

        peer_pool_.SetDestructor(destructor);
    }

    void TcpServer::Impl::Listen(Port port)
    {
        if (acceptor_ == nullptr)
        {
            acceptor_ = std::make_unique<asio::ip::tcp::acceptor>(io_context_, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), port));
        }

        Accept();
    }

    bool TcpServer::Impl::Send(NetID net_id, const char * data, size_t data_size)
    {
        std::lock_guard<std::mutex> lock(mutex_peers_);
        auto peer = peers_[net_id];
        if (peer == nullptr)
        {
            return false;
        }
        peer->Send(data, data_size);
        return true;
    }

    void TcpServer::Impl::Connect(IP ip, Port port)
    {
        if (resolver_ == nullptr)
        {
            resolver_ = std::make_unique<asio::ip::tcp::resolver>(io_context_);
        }

        auto socket = std::make_shared<asio::ip::tcp::socket>(io_context_);
        auto endpoint = resolver_->resolve(ip, std::to_string(port));
        asio::async_connect(*socket, endpoint,
            [this, socket](asio::error_code ec, asio::ip::tcp::endpoint ep)
            {
                if (!ec)
                {
                    AddPeer(std::move(*socket));
                }
            });
    }

    void TcpServer::Impl::Disconnect(NetID net_id)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_peers_);
            peers_.Erase(net_id);
        }

        OnDisconnect(net_id);
    }

    void TcpServer::Impl::RegisterOnConnect(OnConnectFunc func)
    {
        on_connect_ = func;

        if (connect_peers_ == nullptr)
        {
            connect_peers_ = std::make_unique<SwapList<ConnectPeer>>();
        }
    }

    void TcpServer::Impl::RegisterOnRecv(OnRecvFunc func)
    {
        on_recv_ = func;

        if (msgs_ == nullptr)
        {
            msgs_ = std::make_unique<SwapList<TcpMessage>>();
        }
    }

    void TcpServer::Impl::RegisterOnDisconnect(OnDisconnectFunc func)
    {
        on_disconnect_ = func;

        if (disconnect_peers_ == nullptr)
        {
            disconnect_peers_ = std::make_unique<SwapList<NetID>>();
        }
    }

    void TcpServer::Impl::OnConnect(NetID net_id, IP ip, Port port)
    {
        if (connect_peers_ != nullptr)
        {
            connect_peers_->EmplaceBack(ConnectPeer(net_id, ip, port));
        }
    }

    void TcpServer::Impl::OnRecv(NetID net_id, const char * data, size_t data_size)
    {
        if (msgs_ != nullptr)
        {
            msgs_->EmplaceBack(TcpMessage(net_id, data, data_size));
        }
    }

    void TcpServer::Impl::OnDisconnect(NetID net_id)
    {
        if (disconnect_peers_ != nullptr)
        {
            disconnect_peers_->EmplaceBack(std::move(net_id));
        }
    }

    void TcpServer::Impl::Accept()
    {
        acceptor_->async_accept(
            [this](asio::error_code ec, asio::ip::tcp::socket socket)
            {
                if (!ec)
                {
                    AddPeer(std::move(socket));
                }

                Accept();
            });
    }

    void TcpServer::Impl::AddPeer(asio::ip::tcp::socket socket)
    {
        auto net_id = INVALID_NET_ID;
        auto remote_endpoint = socket.remote_endpoint();
        auto ip = remote_endpoint.address().to_string();
        auto port = remote_endpoint.port();

        {
            std::lock_guard<std::mutex> lock(mutex_peers_);
            net_id = (NetID)peers_.Insert(std::move(peer_pool_.Get()));
            auto peer = peers_[net_id];
            peer->Init(net_id, shared_from_this(), std::make_unique<asio::ip::tcp::socket>(std::move(socket)));
            peer->Recv();
        }

        OnConnect(net_id, ip, port);
    }

    //TcpPeer
    TcpServer::Impl::TcpPeer::TcpPeer() : net_id_(INVALID_NET_ID),
        server_(nullptr),
        socket_(nullptr)
    {
    }

    TcpServer::Impl::TcpPeer::~TcpPeer()
    {
    }

    void TcpServer::Impl::TcpPeer::Recv()
    {
        RecvHead();
    }

    void TcpServer::Impl::TcpPeer::Send(const char * data, size_t data_size)
    {
        Message<Buffer> msg(data, data_size);
        asio::async_write(*socket_,
            asio::buffer(msg.Data(), msg.Size()),
            [this](asio::error_code ec, std::size_t size)
            {
                if (!ec)
                {

                }
            });
    }

    void TcpServer::Impl::TcpPeer::Init(const NetID & net_id, std::shared_ptr<TcpServer::Impl> server, std::unique_ptr<asio::ip::tcp::socket> socket)
    {
        net_id_ = net_id;
        server_ = std::move(server);
        socket_ = std::move(socket);
    }

    void TcpServer::Impl::TcpPeer::Release()
    {
        net_id_ = INVALID_NET_ID;
        server_ = nullptr;
        socket_ = nullptr;
    }

    void TcpServer::Impl::TcpPeer::RecvHead()
    {
        msg_.ResetData();
        asio::async_read(*socket_,
            asio::buffer(msg_.Data(), MESSAGE_HEAD_SIZE),
            [this](asio::error_code ec, std::size_t size)
            {
                if (!ec)
                {
                    msg_.DecodeHead();
                    RecvBody();
                }
                else
                {
                    server_->Disconnect(net_id_);
                }
            });
    }

    void TcpServer::Impl::TcpPeer::RecvBody()
    {
        asio::async_read(*socket_,
            asio::buffer(msg_.Body(), msg_.BodySize()),
            [this](asio::error_code ec, std::size_t size)
            {
                if (!ec)
                {
                    server_->OnRecv(net_id_, msg_.Body(), msg_.BodySize());
                    RecvHead();
                }
                else
                {
                    server_->Disconnect(net_id_);
                }
            });
    }

    //TcpServer
    TcpServer::TcpServer(asio::io_context & io_context) : Unit(),
        impl_(std::make_shared<Impl>(io_context))
    {

    }

    TcpServer::~TcpServer()
    {
    }

    bool TcpServer::Start()
    {
        return true;
    }

    void TcpServer::Update(clock_t interval)
    {
        impl_->Update(interval);
    }

    void TcpServer::Stop()
    {
    }

    void TcpServer::Listen(Port port)
    {
        impl_->Listen(port);
    }

    bool TcpServer::Send(NetID net_id, const char * data, size_t data_size)
    {
        return impl_->Send(net_id, data, data_size);
    }

    void TcpServer::Connect(IP ip, Port port)
    {
        impl_->Connect(ip, port);
    }

    void TcpServer::Disconnect(NetID net_id)
    {
        impl_->Disconnect(net_id);
    }

    void TcpServer::RegisterOnConnect(OnConnectFunc func)
    {
        impl_->RegisterOnConnect(func);
    }

    void TcpServer::RegisterOnRecv(OnRecvFunc func)
    {
        impl_->RegisterOnRecv(func);
    }

    void TcpServer::RegisterOnDisconnect(OnDisconnectFunc func)
    {
        impl_->RegisterOnDisconnect(func);
    }
}
