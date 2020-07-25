// Copyright (c) 2019-2020 TungFai Fong 

#include "tcp_server.h"
#include "tcp_server_impl.h"

namespace usrv
{
    //TcpServerImpl
    TcpServer::Impl::Impl(asio::io_context & io_context, clock_t io_interval) : io_context_(io_context),
        acceptor_(nullptr),
        resolver_(nullptr),
        io_interval_(io_interval),
        io_timer_(io_context),
        peer_pool_(),
        connected_peers_(),
        recv_msgs_(),
        disconnected_peers_(),
        on_connect_(nullptr),
        on_recv_(nullptr),
        on_disconnect_(nullptr),
        send_msgs_(),
        to_disconnect_peers_()
    {
        InitPool();
    }

    TcpServer::Impl::~Impl()
    {
    }

    bool TcpServer::Impl::Start()
    {
        IOUpdate();
        return true;
    }

    void TcpServer::Impl::Update(clock_t interval)
    {
        if (on_connect_ != nullptr)
        {
            static const auto connect_peers_each_func = [this](const ConnectedPeer & peer)
            {
                on_connect_(peer.net_id_, peer.ip_, peer.port_);
            };
            connected_peers_.ForEach(connect_peers_each_func);
        }

        if (on_recv_ != nullptr)
        {
            static const auto recv_msgs_each_func = [this](const TcpMessage & msg)
            {
                on_recv_(msg.GetNetID(), msg.Data(), msg.Size());
            };
            recv_msgs_.ForEach(recv_msgs_each_func);
        }

        if (on_disconnect_ != nullptr)
        {
            static const auto disconnect_peers_each_func = [this](const NetID & peer)
            {
                on_disconnect_(peer);
            };
            disconnected_peers_.ForEach(disconnect_peers_each_func);
        }
    }

    void TcpServer::Impl::Stop()
    {
        io_timer_.cancel();
    }

    void TcpServer::Impl::IOUpdate()
    {
        static const auto send_msgs_each_func = [this](const TcpMessage & msg)
        {
            auto peer = peers_[msg.GetNetID()];
            if (peer)
            {
                peer->Send(msg.Data(), msg.Size());
            }
        };
        send_msgs_.ForEach(send_msgs_each_func);

        static const auto to_disconnect_peers_each_func = [this](const NetID & peer)
        {
            OnDisconnect(peer);
            peers_.Erase(peer);
        };
        to_disconnect_peers_.ForEach(to_disconnect_peers_each_func);

        io_timer_.expires_after(std::chrono::milliseconds(io_interval_));
        io_timer_.async_wait(std::bind(&TcpServer::Impl::IOUpdate, this));
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

    void TcpServer::Impl::Send(NetID net_id, const char * data, size_t data_size)
    {
        send_msgs_.EmplaceBack(TcpMessage(net_id, data, data_size));
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
        to_disconnect_peers_.EmplaceBack(std::move(net_id));
    }

    void TcpServer::Impl::RegisterOnConnect(OnConnectFunc func)
    {
        on_connect_ = func;
    }

    void TcpServer::Impl::RegisterOnRecv(OnRecvFunc func)
    {
        on_recv_ = func;
    }

    void TcpServer::Impl::RegisterOnDisconnect(OnDisconnectFunc func)
    {
        on_disconnect_ = func;
    }

    void TcpServer::Impl::OnConnect(NetID net_id, IP ip, Port port)
    {
        connected_peers_.EmplaceBack(ConnectedPeer(net_id, ip, port));
    }

    void TcpServer::Impl::OnRecv(NetID net_id, const char * data, size_t data_size)
    {
        recv_msgs_.EmplaceBack(TcpMessage(net_id, data, data_size));
    }

    void TcpServer::Impl::OnDisconnect(NetID net_id)
    {
        disconnected_peers_.EmplaceBack(std::move(net_id));
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
        auto remote_endpoint = socket.remote_endpoint();
        auto ip = remote_endpoint.address().to_string();
        auto port = remote_endpoint.port();

        auto net_id = (NetID)peers_.Insert(std::move(peer_pool_.Get()));
        auto peer = peers_[net_id];
        peer->Init(net_id, shared_from_this(), std::move(socket));
        peer->Recv();

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

    void TcpServer::Impl::TcpPeer::Init(const NetID & net_id, std::shared_ptr<TcpServer::Impl> server, asio::ip::tcp::socket socket)
    {
        net_id_ = net_id;
        server_ = std::move(server);
        socket_ = std::make_unique<asio::ip::tcp::socket>(std::move(socket));
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
    TcpServer::TcpServer(asio::io_context & io_context, clock_t io_interval) : Unit(),
        impl_(std::make_shared<Impl>(io_context, io_interval))
    {

    }

    TcpServer::~TcpServer()
    {
    }

    bool TcpServer::Start()
    {
        return impl_->Start();
    }

    void TcpServer::Update(clock_t interval)
    {
        impl_->Update(interval);
    }

    void TcpServer::Stop()
    {
        impl_->Stop();
    }

    void TcpServer::Listen(Port port)
    {
        impl_->Listen(port);
    }

    void TcpServer::Send(NetID net_id, const char * data, size_t data_size)
    {
        impl_->Send(net_id, data, data_size);
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
