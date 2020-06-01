// Copyright (c) 2019-2020 TungFai Fong 

#include "message.h"

#include <cstring>

namespace usrv
{
    Message::Message() :buffer_()
    {
    }

    Message::Message(const char * body, size_t body_size) : buffer_()
    {
        SetBodySize(body_size);
        EncodeHead();
        buffer_.Set(body, BodySize(), MESSAGE_HEAD_SIZE);
    }

    void Message::ResetData()
    {
        buffer_.ResetData();
    }

    void Message::SetBodySize(size_t size)
    {
        size = (size <= 0) ? 0 : (size > MESSAGE_BODY_SIZE ? MESSAGE_BODY_SIZE : size);
        buffer_.Resize(MESSAGE_HEAD_SIZE + size);
    }

    void Message::EncodeHead()
    {
        auto body_size = static_cast<uint16_t>(BodySize());
        memcpy(Data(), &body_size, MESSAGE_HEAD_SIZE);
    }

    void Message::DecodeHead()
    {
        auto body_size = static_cast<uint16_t>(0);
        memcpy(&body_size, Data(), MESSAGE_HEAD_SIZE);
        SetBodySize(body_size);
    }
}
