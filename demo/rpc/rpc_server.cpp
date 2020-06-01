// Copyright (c) 2019-2020 TungFai Fong 

#include <iostream>
#include <functional>

#include "rpc_test.pb.h"

#include "unit_manager.h"
#include "rpc/rpc_server.h"

static constexpr usrv::RpcStubID TEST_STUB_ID = 1;
static constexpr usrv::RpcFuncID TEST_FUNC_ID = 1;

class ServerStub : public usrv::RpcServerStub
{
public:
    ServerStub(std::shared_ptr<usrv::RpcServer> server) : RpcServerStub(TEST_STUB_ID, server)
    {
        Register(TEST_FUNC_ID, std::bind(&ServerStub::Sum, this, std::placeholders::_1, std::placeholders::_2));
    }

public:
    usrv::RpcResult Sum(const usrv::RpcParam & in_param, usrv::RpcParam * out_param)
    {
        ::pb::test::Request req;
        if (!req.ParseFromArray(in_param.Data(), (int)in_param.Size()))
        {
            return usrv::RpcResult::INVALID_IN_PARAM;
        }

        ::pb::test::Response res;
        res.set_c(req.a() + req.b());
        std::cout << "Sum " << req.a() << "+" << req.b() << std::endl;
        out_param->Reset(res.SerializeAsString().c_str(), res.ByteSizeLong());

        if (out_param->Size() != res.ByteSizeLong())
        {
            return usrv::RpcResult::INVALID_OUT_PARAM;
        }

        return usrv::RpcResult::SUCCESS;
    }
};

bool run_rpc_server(int port)
{
    usrv::UnitManager mgr(10);

    auto rpc_server = std::make_shared<usrv::RpcServer>(mgr.IOContext());

    mgr.Register("server", std::move(rpc_server));

    auto server = std::dynamic_pointer_cast<usrv::RpcServer>(mgr.Get("server"));

    auto stub = std::make_shared<ServerStub>(server);
    server->RegisterServer(stub);

    server->Listen(port);

    mgr.Run();

    return true;
}