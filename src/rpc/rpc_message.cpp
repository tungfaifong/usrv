// Copyright (c) 2019-2020 TungFai Fong 

#include "rpc_message.h"

namespace usrv
{
    RpcMessage::RpcMessage(RpcType type, RpcStubID client_id, RpcStubID server_id, RpcFuncID func_id) :type_(type),
        client_id_(client_id),
        server_id_(server_id),
        func_id_(func_id),
        param_()
    {
    }
}
