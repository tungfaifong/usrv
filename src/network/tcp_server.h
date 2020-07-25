// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_TCP_SERVER_H
#define USRV_TCP_SERVER_H

#include "asio.hpp"

#include "common/common_def.h"
#include "unit.h"

namespace usrv
{
    class TcpServer : public Unit
    {
    public:
        TcpServer(asio::io_context & io_context, clock_t io_interval);
        virtual ~TcpServer();

        virtual bool Start() override final;
        virtual void Update(clock_t interval) override final;
        virtual void Stop() override final;

    public:
        void Listen(Port port);
        void Send(NetID net_id, const char * data, size_t data_size);
        void Connect(IP ip, Port port);
        void Disconnect(NetID net_id);

        using OnConnectFunc = std::function<void(NetID, IP, Port)>;
        void RegisterOnConnect(OnConnectFunc func);

        using OnRecvFunc = std::function<void(NetID, const char *, size_t)>;
        void RegisterOnRecv(OnRecvFunc func);

        using OnDisconnectFunc = std::function<void(NetID)>;
        void RegisterOnDisconnect(OnDisconnectFunc func);

    private:
        class Impl;
        std::shared_ptr<Impl> impl_;
    };
}

#endif // USRV_TCP_SERVER_H
