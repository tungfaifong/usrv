// Copyright (c) 2019-2020 TungFai Fong 

#include <iostream>

#include "asio.hpp"

#include "network/message.h"

class Client
{
public:
    Client(asio::io_context & io_context, const asio::ip::tcp::resolver::results_type& endpoints) : io_context_(io_context),
        socket_(io_context)
    {
        Connect(endpoints);
    }

    void Connect(const asio::ip::tcp::resolver::results_type& endpoints)
    {
        asio::async_connect(socket_, endpoints,
            [this](asio::error_code ec, asio::ip::tcp::endpoint ep)
            {
                if (!ec)
                {
                    RecvHead();
                }
                else
                {
                    socket_.close();
                }
            });
    }

    void RecvHead()
    {
        msg_.ResetData();
        asio::async_read(socket_,
            asio::buffer(msg_.Data(), usrv::MESSAGE_HEAD_SIZE),
            [this](asio::error_code ec, std::size_t size)
            {
                if (!ec)
                {
                    msg_.DecodeHead();
                    RecvBody();
                }
                else
                {
                    socket_.close();
                }
            });
    }

    void RecvBody()
    {
        asio::async_read(socket_,
            asio::buffer(msg_.Body(), msg_.BodySize()),
            [this](asio::error_code ec, std::size_t size)
            {
                if (!ec)
                {
                    std::cout << msg_.Body() << std::endl;
                    RecvHead();
                }
                else
                {
                    socket_.close();
                }
            });
    }

    void Send(const char * data, int data_size)
    {
        usrv::Message msg(data, data_size);
        asio::async_write(socket_,
            asio::buffer(msg.Data(), msg.Size()),
            [this](asio::error_code ec, std::size_t size)
            {
                if (!ec)
                {

                }
            });
    }
private:
    asio::io_context & io_context_;
    asio::ip::tcp::socket socket_;
    usrv::Message msg_;
};

bool run_tcp_client(std::string host, std::string port, int client_num)
{
    asio::io_context io_context;
    asio::ip::tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(host, port);
    std::vector<std::unique_ptr<Client>> clients;
    for (auto i = 0; i < client_num; ++i)
    {
        clients.emplace_back(std::make_unique<Client>(io_context, endpoints));
    }

    std::thread t([&io_context]() {
        io_context.run();
        });

    while (true)
    {
        for (auto & client : clients)
        {
            const char * str = "echo";
            client->Send(str, (int)strlen(str));
        }
    }

    t.join();

    return true;
}