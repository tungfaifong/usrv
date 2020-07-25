// Copyright (c) 2019-2020 TungFai Fong 

#include <iostream>

#include "unit_manager.h"
#include "network/tcp_server.h"

bool run_tcp_server(int port)
{
    usrv::UnitManager mgr(10);

    auto tcp_server = std::make_shared<usrv::TcpServer>(mgr.IOContext(), mgr.Interval());

    tcp_server->RegisterOnRecv([&](usrv::NetID net_id, const char * data, size_t data_size) {
        std::cout << "net_id:" << net_id << " data:" << data << std::endl;

        auto server = std::dynamic_pointer_cast<usrv::TcpServer>(mgr.Get("server"));
        server->Send(net_id, data, data_size);
        });

    tcp_server->RegisterOnConnect([&](usrv::NetID net_id, std::string ip, usrv::Port port) {
        std::cout << "net_id:" << net_id << " ip:" << ip << " port:" << port << " connect to tcp server" << std::endl;
        });

    tcp_server->RegisterOnDisconnect([](usrv::NetID net_id) {
        std::cout << "net_id:" << net_id << " disconnect from tcp server" << std::endl;
        });

    mgr.Register("server", std::move(tcp_server));

    auto server = std::dynamic_pointer_cast<usrv::TcpServer>(mgr.Get("server"));

    server->Listen(port);
    
    mgr.Run();

    return true;
}