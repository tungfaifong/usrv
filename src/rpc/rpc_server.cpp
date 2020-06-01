// Copyright (c) 2019-2020 TungFai Fong 

#include "rpc_server.h"
#include "rpc_server_impl.h"

#include "network/tcp_server.h"
#include "rpc_message.h"

namespace usrv
{
    //RpcServerImpl
    RpcServer::Impl::Impl(asio::io_context & io_context) : tcp_server_(std::make_shared<TcpServer>(io_context)),
        server_net_id_(INVALID_NET_ID),
        server_ip_(),
        server_port_(0)
    {
        InitTcpServer();
    }

    RpcServer::Impl::~Impl()
    {
    }

    bool RpcServer::Impl::Start()
    {
        return tcp_server_->Start();
    }

    void RpcServer::Impl::Update(clock_t interval)
    {
        tcp_server_->Update(interval);

        for (uint32_t i = 0; i < client_stubs_.Size(); ++i)
        {
            auto stub = client_stubs_[i];
            if (stub == nullptr)
            {
                continue;
            }
            (*stub)->timeout_ -= interval;
            if ((*stub)->timeout_ <= 0)
            {
                (*stub)->OnTimeout();
                client_stubs_.Erase(i);
            }
        }
    }

    void RpcServer::Impl::Stop()
    {
        tcp_server_->Stop();
    }

    void RpcServer::Impl::InitTcpServer()
    {
        tcp_server_->RegisterOnRecv([this](usrv::NetID net_id, const char * data, size_t data_size) {
            auto msg = (const RpcMessage *)data;
            auto msg_type = msg->Type();
            if (msg_type == RpcType::SERVER)
            {
                OnCall(net_id, msg->ClientID(), msg->ServerID(), msg->FuncID(), msg->Param());
            }
            else if (msg_type == RpcType::CLIENT)
            {
                OnResponse(msg->ClientID(), msg->ServerID(), msg->FuncID(), msg->Result(), msg->Param());
            }
            });

        tcp_server_->RegisterOnConnect([this](NetID net_id, std::string ip, Port port) {
            if (server_ip_ == ip && server_port_ == port)
            {
                server_net_id_ = net_id;
            }
            });

        tcp_server_->RegisterOnDisconnect([this](NetID net_id) {
            if (server_net_id_ == net_id)
            {
                server_net_id_ = INVALID_NET_ID;
            }
            });
    }

    void RpcServer::Impl::Listen(Port port)
    {
        tcp_server_->Listen(port);
    }

    void RpcServer::Impl::Connect(std::string ip, Port port)
    {
        server_ip_ = ip;
        server_port_ = port;
        tcp_server_->Connect(ip, port);
    }

    bool RpcServer::Impl::RegisterServer(std::shared_ptr<RpcServerStub> stub)
    {
        const auto & stub_id = stub->ID();
        if (server_stubs_.find(stub_id) != server_stubs_.end())
        {
            return false;
        }

        server_stubs_.insert(std::make_pair(stub_id, std::move(stub)));

        return true;
    }

    uint32_t RpcServer::Impl::RegisterClient(std::shared_ptr<RpcClientStub> stub)
    {
        return client_stubs_.Insert(std::move(stub));
    }

    bool RpcServer::Impl::Call(const RpcStubID & client_id, const RpcStubID & server_id, const RpcFuncID & func_id, const char * param, size_t param_size)
    {
        if (server_net_id_ == INVALID_NET_ID)
        {
            return false;
        }

        if (param_size > RPC_PARAM_LENGTH)
        {
            return false;
        }

        RpcMessage msg(RpcType::SERVER, client_id, server_id, func_id);
        msg.Param().Reset(param, param_size);
        return Send(server_net_id_, (const char *)&msg, msg.Size());
    }

    bool RpcServer::Impl::Send(NetID net_id, const char * data, size_t data_size)
    {
        return tcp_server_->Send(net_id, data, data_size);
    }

    void RpcServer::Impl::OnCall(const NetID & net_id, const RpcStubID & client_id, const RpcStubID & server_id, const RpcFuncID & func_id, const RpcParam & param)
    {
        auto code = RpcResult::SUCCESS;

        RpcMessage msg(RpcType::CLIENT, client_id, server_id, func_id);

        auto iter = server_stubs_.find(server_id);
        if (iter == server_stubs_.end())
        {
            code = RpcResult::INVALID_STUB;
        }
        else
        {
            code = iter->second->Call(func_id, param, &msg.Param());
        }

        msg.SetResult(code);

        Send(net_id, (const char *)&msg, (int)msg.Size());
    }

    void RpcServer::Impl::OnResponse(const RpcStubID & client_id, const RpcStubID & server_id, const RpcFuncID & func_id, const RpcResult & result, const RpcParam & param)
    {
        auto iter = client_stubs_[client_id];
        if (iter == nullptr)
        {
            return;
        }

        (*iter)->OnResponse(result, param);

        client_stubs_.Erase(client_id);
    }

    //RpcServerStub
    RpcServerStub::RpcServerStub(const RpcStubID & id, std::shared_ptr<RpcServer> server) : id_(id),
        server_(server)
    {
    }

    RpcServerStub::~RpcServerStub()
    {
    }

    bool RpcServerStub::Register(const RpcFuncID & func_id, StubFunc func)
    {
        if (funcs_.find(func_id) != funcs_.end())
        {
            return false;
        }

        funcs_.insert(std::make_pair(func_id, func));

        return true;
    }

    RpcResult RpcServerStub::Call(const RpcFuncID & func_id, const RpcParam & in_param, RpcParam * out_param)
    {
        auto iter = funcs_.find(func_id);
        if (iter == funcs_.end())
        {
            return RpcResult::INVALID_FUNC;
        }

        return iter->second(in_param, out_param);
    }

    //RpcClientStub
    RpcClientStub::RpcClientStub(std::shared_ptr<RpcServer> server, clock_t timeout) : id_(INVALID_RPC_STUB_ID),
        timeout_(timeout),
        server_(server)
    {
    }

    RpcClientStub::~RpcClientStub()
    {
    }

    bool RpcClientStub::Call(RpcStubID server_id, RpcFuncID func_id, const char * param, size_t param_size)
    {
        if (server_ == nullptr)
        {
            return false;
        }

        return server_->Call(id_, server_id, func_id, param, param_size);
    }

    //RpcServer
    RpcServer::RpcServer(asio::io_context & io_context) : Unit(),
        impl_(std::make_unique<Impl>(io_context))
    {
    }

    RpcServer::~RpcServer()
    {
    }

    bool RpcServer::Start()
    {
        return impl_->Start();
    }

    void RpcServer::Update(clock_t interval)
    {
        impl_->Update(interval);
    }

    void RpcServer::Stop()
    {
        impl_->Stop();
    }

    void RpcServer::Listen(Port port)
    {
        impl_->Listen(port);
    }

    void RpcServer::Connect(std::string ip, Port port)
    {
        impl_->Connect(ip, port);
    }

    bool RpcServer::RegisterServer(std::shared_ptr<RpcServerStub> stub)
    {
        return impl_->RegisterServer(stub);
    }

    uint32_t RpcServer::RegisterClient(std::shared_ptr<RpcClientStub> stub)
    {
        return impl_->RegisterClient(stub);
    }

    bool RpcServer::Call(const RpcStubID & client_id, const RpcStubID & server_id, const RpcFuncID & func_id, const char * param, size_t param_size)
    {
        return impl_->Call(client_id, server_id, func_id, param, param_size);
    }
}
