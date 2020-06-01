// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_RPC_MESSAGE
#define USRV_RPC_MESSAGE

#include "common/common_def.h"

namespace usrv
{
    class RpcMessage
    {
    public:
        RpcMessage(RpcType type = RpcType::INVALID, RpcStubID client_id = INVALID_RPC_STUB_ID, RpcStubID server_id = INVALID_RPC_STUB_ID, RpcFuncID func_id = INVALID_RPC_FUNC_ID);

        RpcType Type() const { return type_; }
        void SetResult(RpcResult result) { result_ = result; }
        RpcResult Result() const { return result_; }
        RpcStubID ClientID() const { return client_id_; }
        RpcStubID ServerID() const { return server_id_; }
        RpcFuncID FuncID() const { return func_id_; }
        const RpcParam & Param() const { return param_; }
        RpcParam & Param() { return const_cast<RpcParam&>(static_cast<const RpcMessage &>(*this).Param()); }

        size_t Size() const { return sizeof(RpcMessage) - (RPC_PARAM_LENGTH - param_.Size()) * sizeof(char); }

    private:
        RpcType type_;
        RpcResult result_;
        RpcStubID client_id_;
        RpcStubID server_id_;
        RpcFuncID func_id_;
        RpcParam param_;
    };
}

#endif // USRV_RPC_MESSAGE
