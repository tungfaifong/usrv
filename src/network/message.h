// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_MESSAGE
#define USRV_MESSAGE

#include "common/common_def.h"

namespace usrv
{
    class Message
    {
    public:
        Message();
        Message(const char * body, size_t body_size);

        void ResetData();

        const char * Data() const { return buffer_.Data(); }
        char * Data() { return const_cast<char *>(static_cast<const Message &>(*this).Data()); }
        size_t Size() { return buffer_.Size(); }

        const char * Body() const { return Data() + MESSAGE_HEAD_SIZE; }
        char * Body() { return const_cast<char *>(static_cast<const Message &>(*this).Body()); }
        size_t BodySize() { return Size() - MESSAGE_HEAD_SIZE; }
        void SetBodySize(size_t size);

        void EncodeHead();
        void DecodeHead();

    private:
        FixedBuffer<MESSAGE_HEAD_SIZE + MESSAGE_BODY_SIZE> buffer_;
    };
}

#endif // USRV_MESSAGE
