// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <string>

#define NAMESPACE_OPEN namespace \
usrv \
{
#define NAMESPACE_CLOSE \
}

NAMESPACE_OPEN

using intvl_t = uint64_t;

using NETID = uint32_t;
using IP = std::string;
using PORT = uint16_t;

static constexpr NETID INVALID_NET_ID = 0;

static constexpr uint16_t MESSAGE_HEAD_SIZE = sizeof(uint16_t);
static constexpr uint16_t MESSAGE_BODY_SIZE = UINT16_MAX - MESSAGE_HEAD_SIZE;

NAMESPACE_CLOSE

#endif // COMMON_H
