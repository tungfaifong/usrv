// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_RPC_SERVER_H
#define USRV_RPC_SERVER_H

#include <map>
#include <time.h>

#include "asio.hpp"

#include "common/common_def.h"
#include "unit.h"

namespace usrv
{
    class RpcServer;

    class RpcServerStub
    {
    public:
        RpcServerStub(const RpcStubID & id, std::shared_ptr<RpcServer> server);
        virtual ~RpcServerStub();

    public:
        using StubFunc = std::function<RpcResult(const RpcParam &, RpcParam *)>;
        bool Register(const RpcFuncID & func_id, StubFunc func);
        RpcResult Call(const RpcFuncID & func_id, const RpcParam & in_param, RpcParam * out_param);

    public:
        RpcStubID ID() { return id_; }

    private:
        RpcStubID id_;
        std::shared_ptr<RpcServer> server_;
        std::map<RpcFuncID, StubFunc> funcs_;
    };

    class RpcClientStub
    {
    public:
        static constexpr clock_t RPC_DEFAULT_TIMEOUT = 30 * 1000;
        friend class RpcServer;

    public:
        RpcClientStub(std::shared_ptr<RpcServer> server, clock_t timeout = RPC_DEFAULT_TIMEOUT);
        virtual ~RpcClientStub();

    public:
        virtual void OnResponse(const RpcResult & result, const RpcParam & param) {}
        virtual void OnTimeout() {}

    public:
        void SetID(RpcStubID id) { id_ = id; }

    protected:
        bool Call(RpcStubID server_id, RpcFuncID func_id, const char * param, size_t param_size);

    private:
        RpcStubID id_;
        clock_t timeout_;
        std::shared_ptr<RpcServer> server_;
    };

    class RpcServer : public Unit
    {
    public:
        RpcServer(asio::io_context & io_context, clock_t io_interval);
        virtual ~RpcServer();

        virtual bool Start() override final;
        virtual void Update(clock_t interval) override final;
        virtual void Stop() override final;

    public:
        void Listen(Port port);
        void Connect(std::string ip, Port port);

        bool RegisterServer(std::shared_ptr<RpcServerStub> stub);
        uint32_t RegisterClient(std::shared_ptr<RpcClientStub> stub);

        bool Call(const RpcStubID & client_id, const RpcStubID & server_id, const RpcFuncID & func_id, const char * param, size_t param_size);

    private:
        class Impl;
        std::unique_ptr<Impl> impl_;
    };
}

#endif // USRV_RPC_SERVER_H
