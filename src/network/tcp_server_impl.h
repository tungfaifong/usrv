// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_TCP_SERVER_IMPL_H
#define USRV_TCP_SERVER_IMPL_H

#include <map>

#include "common/object_pool.h"
#include "common/swap_list.h"
#include "common/object_list.h"
#include "message.h"
#include "tcp_message.h"

namespace usrv
{
    class TcpServer::Impl : public std::enable_shared_from_this<TcpServer::Impl>
    {
    public:
        Impl(asio::io_context & io_context, clock_t io_interval);
        ~Impl();

        bool Start();
        void Update(clock_t interval);
        void Stop();
        void IOUpdate();

    private:
        void InitPool();

    public:
        void Listen(Port port);
        void Send(NetID net_id, const char * data, size_t data_size);
        void Connect(IP ip, Port port);
        void Disconnect(NetID net_id);

        void RegisterOnConnect(OnConnectFunc func);
        void RegisterOnRecv(OnRecvFunc func);
        void RegisterOnDisconnect(OnDisconnectFunc func);

    private:
        void OnConnect(NetID net_id, IP ip, Port port);
        void OnRecv(NetID net_id, const char * data, size_t data_size);
        void OnDisconnect(NetID net_id);

        void Accept();

        void AddPeer(asio::ip::tcp::socket socket);

    private:
        asio::io_context & io_context_;
        std::unique_ptr<asio::ip::tcp::acceptor> acceptor_;
        std::unique_ptr<asio::ip::tcp::resolver> resolver_;

        clock_t io_interval_;
        asio::steady_timer io_timer_;

        class TcpPeer;
        ObjectPool<TcpPeer> peer_pool_;
        ObjectList<TcpPeer> peers_;

        //IO2MAIN
        struct ConnectedPeer
        {
            ConnectedPeer(NetID net_id, IP ip, Port port) : net_id_(net_id), ip_(std::move(ip)), port_(port) {}
            NetID net_id_;
            IP ip_;
            Port port_;
        };
        SwapList<ConnectedPeer> connected_peers_;
        SwapList<TcpMessage> recv_msgs_;
        SwapList<NetID> disconnected_peers_;

        OnConnectFunc on_connect_;
        OnRecvFunc on_recv_;
        OnDisconnectFunc on_disconnect_;

        //MAIN2IO
        SwapList<TcpMessage> send_msgs_;
        SwapList<NetID> to_disconnect_peers_;
    };

    class TcpServer::Impl::TcpPeer
    {
    public:
        TcpPeer();
        virtual ~TcpPeer();

    public:
        void Recv();
        void Send(const char * data, size_t data_size);

    public:
        void Init(const NetID & net_id, std::shared_ptr<TcpServer::Impl> server, asio::ip::tcp::socket socket);
        void Release();

    private:
        void RecvHead();
        void RecvBody();

    private:
        NetID net_id_;
        std::shared_ptr<TcpServer::Impl> server_;
        std::unique_ptr<asio::ip::tcp::socket> socket_;
        Message<FixedBuffer<MESSAGE_SIZE>> msg_;
    };
}

#endif // USRV_TCP_SERVER_IMPL_H
