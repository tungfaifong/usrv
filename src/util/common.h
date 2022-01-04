// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_COMMON_H
#define USRV_COMMON_H

#include <stdint.h>
#include <string>

namespace usrv
{

using intvl_t = uint64_t;

using NETID = uint32_t;
using IP = std::string;
using PORT = uint16_t;

static constexpr uint32_t SPSC_QUEUE_BLOCK_NUM = 1024 * 1024;

static constexpr NETID INVALID_NET_ID = -1;

static constexpr uint16_t MESSAGE_HEAD_SIZE = sizeof(uint16_t);
static constexpr uint16_t MESSAGE_BODY_SIZE = UINT16_MAX - MESSAGE_HEAD_SIZE;

}

#endif // USRV_COMMON_H
