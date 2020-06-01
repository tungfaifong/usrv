// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_RPC_SERVER_IMPL_H
#define USRV_RPC_SERVER_IMPL_H

#include "common/object_list.hpp"

namespace usrv
{
    class TcpServer;

    class RpcServer::Impl
    {
    public:
        Impl(asio::io_context & io_context);
        virtual ~Impl();

        bool Start();
        void Update(clock_t interval);
        void Stop();

    private:
        void InitTcpServer();

    public:
        void Listen(Port port);
        void Connect(std::string ip, Port port);

        bool RegisterServer(std::shared_ptr<RpcServerStub> stub);
        uint32_t RegisterClient(std::shared_ptr<RpcClientStub> stub);

        bool Call(const RpcStubID & client_id, const RpcStubID & server_id, const RpcFuncID & func_id, const char * param, size_t param_size);

    private:
        bool Send(NetID net_id, const char * data, size_t data_size);

        void OnCall(const NetID & net_id, const RpcStubID & client_id, const RpcStubID & server_id, const RpcFuncID & func_id, const RpcParam & param);
        void OnResponse(const RpcStubID & client_id, const RpcStubID & server_id, const RpcFuncID & func_id, const RpcResult & result, const RpcParam & param);

    private:
        std::shared_ptr<TcpServer> tcp_server_;
        NetID server_net_id_;
        std::string server_ip_;
        Port server_port_;
        std::map<RpcStubID, std::shared_ptr<RpcServerStub>> server_stubs_;
        ObjectList<std::shared_ptr<RpcClientStub>> client_stubs_;
    };
}

#endif // USRV_RPC_SERVER_IMPL_H
