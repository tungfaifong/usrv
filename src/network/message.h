// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_MESSAGE
#define USRV_MESSAGE

#include "common/common_def.h"

namespace usrv
{
    template<typename T>
    class Message
    {
    public:
        Message() : buffer_() {}

        Message(const char * body, size_t body_size) : buffer_()
        {
            SetBodySize(body_size);
            EncodeHead();
            buffer_.Set(body, BodySize(), MESSAGE_HEAD_SIZE);
        }

        void ResetData()
        {
            buffer_.ResetData();
        }

        const char * Data() const { return buffer_.Data(); }
        char * Data() { return const_cast<char *>(static_cast<const Message &>(*this).Data()); }
        size_t Size() { return buffer_.Size(); }

        const char * Body() const { return Data() + MESSAGE_HEAD_SIZE; }
        char * Body() { return const_cast<char *>(static_cast<const Message &>(*this).Body()); }
        size_t BodySize() { return Size() - MESSAGE_HEAD_SIZE; }

        void SetBodySize(size_t size)
        {
            size = (size <= 0) ? 0 : (size > MESSAGE_BODY_SIZE ? MESSAGE_BODY_SIZE : size);
            buffer_.Resize(MESSAGE_HEAD_SIZE + size);
        }

        void EncodeHead()
        {
            auto body_size = BodySize();
            memcpy(Data(), &body_size, MESSAGE_HEAD_SIZE);
        }

        void DecodeHead()
        {
            auto body_size = 0;
            memcpy(&body_size, Data(), MESSAGE_HEAD_SIZE);
            SetBodySize(body_size);
        }

    private:
        T buffer_;
    };
}

#endif // USRV_MESSAGE
