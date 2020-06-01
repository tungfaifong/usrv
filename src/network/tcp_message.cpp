// Copyright (c) 2019-2020 TungFai Fong 

#include "tcp_message.h"

#include <utility>

namespace usrv
{
    TcpMessage::TcpMessage(NetID net_id, const char * data, size_t data_size) :net_id_(net_id), buffer_(data, data_size)
    {
    }

    TcpMessage::TcpMessage(TcpMessage && r) : net_id_(r.net_id_),
        buffer_(std::move(r.buffer_))
    {
    }
}
