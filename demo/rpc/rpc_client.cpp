// Copyright (c) 2019-2020 TungFai Fong 

#include <iostream>
#include <functional>

#include "rpc_test.pb.h"

#include "unit_manager.h"
#include "rpc/rpc_server.h"

static constexpr usrv::RpcStubID TEST_STUB_ID = 1;
static constexpr usrv::RpcFuncID TEST_FUNC_ID = 1;

class ClientStub : public usrv::RpcClientStub
{
public:
    ClientStub(std::shared_ptr<usrv::RpcServer> server) : RpcClientStub(server)
    {
    }

public:
    bool Sum(int a, int b)
    {
        ::pb::test::Request req;
        req.set_a(a);
        req.set_b(b);
        return Call(TEST_STUB_ID, TEST_FUNC_ID, req.SerializeAsString().c_str(), req.ByteSizeLong());
    }

    virtual void OnResponse(const usrv::RpcResult & result, const usrv::RpcParam & param) override
    {
        std::cout << "result:" << (int)result;
        ::pb::test::Response res;
        if (res.ParseFromArray(param.Data(), (int)param.Size()))
        {
            std::cout << " SumRes:" << res.c() << std::endl;
        }
    }

    virtual void OnTimeout() override
    {
        std::cout << "Sum Timeout" << std::endl;
    }
};

class Game : public usrv::Unit
{
public:
    Game(usrv::UnitManager * mgr) : Unit(), mgr_(mgr) {}
    virtual ~Game() {}

    virtual bool Start() override
    {
        return true;
    }

    virtual void Update(clock_t interval) override
    {
        auto server = std::dynamic_pointer_cast<usrv::RpcServer>(mgr_->Get("server"));

        auto stub = std::make_shared<ClientStub>(server);

        stub->SetID(server->RegisterClient(stub));
        //stub->Sum(1, 2);
    }

    virtual void Stop() override {}

    usrv::UnitManager * mgr_;
};

bool run_rpc_client(std::string host, int port)
{
    usrv::UnitManager mgr(10);

    auto rpc_server = std::make_shared<usrv::RpcServer>(mgr.IOContext(), mgr.Interval());
    mgr.Register("server", std::move(rpc_server));

    auto game = std::make_shared<Game>(&mgr);
    mgr.Register("game", std::move(game));

    auto server = std::dynamic_pointer_cast<usrv::RpcServer>(mgr.Get("server"));
    server->Connect(host, port);

    mgr.Run();

    return true;
}
