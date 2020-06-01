// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_TCP_MESSAGE
#define USRV_TCP_MESSAGE

#include "common/common_def.h"

namespace usrv
{
    class TcpMessage
    {
    public:
        TcpMessage(NetID net_id, const char * data, size_t data_size);
        TcpMessage(TcpMessage && r);

    public:
        NetID GetNetID() const { return net_id_; }
        const char * Data() const { return buffer_.Data(); }
        size_t Size() const { return buffer_.Size(); }

    private:
        NetID net_id_;
        Buffer buffer_;
    };
}

#endif // USRV_TCP_MESSAGE
