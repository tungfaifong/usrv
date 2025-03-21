// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_COMMON_H
#define USRV_COMMON_H

#include <stdint.h>
#include <string>
#include <unistd.h>

#ifndef NAMESPACE_NAME
#define NAMESPACE_NAME usrv
#endif

#define NAMESPACE_OPEN namespace \
NAMESPACE_NAME \
{
#define NAMESPACE_CLOSE \
}

NAMESPACE_OPEN

static constexpr uint32_t USRV_VERSION = 0001000;

#define Ki * 1024
#define Mi Ki Ki

using intvl_t = uint64_t;
using TID = size_t;
using PEERID = size_t;
using IP = std::string;
using PORT = uint16_t;
using TIMERID = size_t;

static constexpr TID INVALID_TID = SIZE_MAX;
static constexpr PEERID INVALID_PEER_ID = SIZE_MAX;
struct NETID {
	TID tid = INVALID_TID;
	PEERID pid = INVALID_PEER_ID;
	bool operator==(const NETID& other) const {
		return tid == other.tid && pid == other.pid;
	}
};

static constexpr NETID INVALID_NET_ID{INVALID_TID, INVALID_PEER_ID};

extern IP DEFAULT_IP; // {""}
static constexpr PORT DEFAULT_PORT = 0;
static constexpr TIMERID INVALID_TIMER_ID = -1;

static constexpr uint16_t MESSAGE_HEAD_SIZE = sizeof(uint16_t);
static constexpr uint16_t MESSAGE_BODY_SIZE = 65536 - MESSAGE_HEAD_SIZE; //asio::detail::default_max_transfer_size(65536) - MESSAGE_HEAD_SIZE asio限制了一次发送的最大大小

extern std::string PATH_ROOT; // getcwd(NULL, 1024)

void SignalHandler(int signo);
void SignalInit();

NAMESPACE_CLOSE

namespace std {
	template<> 
	struct hash<usrv::NETID> {
		size_t operator()(const usrv::NETID& id) const {
			return (id.tid << 56) | id.pid;
		}
	};
}

#endif // USRV_COMMON_H
