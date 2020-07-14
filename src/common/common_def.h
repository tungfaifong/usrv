// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_COMMON_DEF_H
#define USRV_COMMON_DEF_H

#include <stdint.h>
#include <string>

#include "buffer.h"

namespace usrv
{
    //NETWORK
    using NetID = uint32_t;
    using IP = std::string;
    using Port = uint16_t;

    static constexpr NetID INVALID_NET_ID = -1;     //这里取-1没问题NetID不可能为21亿

    static constexpr uint16_t MESSAGE_HEAD_SIZE = 2;
    static constexpr uint16_t MESSAGE_BODY_SIZE = UINT16_MAX;
    static constexpr size_t MESSAGE_SIZE = MESSAGE_HEAD_SIZE + MESSAGE_BODY_SIZE;

    //RPC
    using RpcStubID = uint32_t;
    using RpcFuncID = uint16_t;

    static constexpr RpcStubID INVALID_RPC_STUB_ID = -1;
    static constexpr RpcFuncID INVALID_RPC_FUNC_ID = -1;

    static constexpr uint16_t RPC_PARAM_LENGTH = MESSAGE_BODY_SIZE - sizeof(int64_t) * 2;
    using RpcParam = FixedBuffer<RPC_PARAM_LENGTH>;

    enum class RpcType : uint8_t
    {
        INVALID,
        SERVER,
        CLIENT,
    };

    enum class RpcResult : uint8_t
    {
        SUCCESS,
        INVALID_STUB,
        INVALID_FUNC,
        INVALID_IN_PARAM,
        INVALID_OUT_PARAM,
    };
}

#endif // USRV_COMMON_DEF_H
