// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_SERVER_INTERFACE_HPP
#define USRV_SERVER_INTERFACE_HPP

#include "unit_manager.h"
#include "units/server_unit.h"
#include "util/common.h"

NAMESPACE_OPEN

namespace server
{

inline NETID Connect(const IP & ip, PORT port)
{
	return std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"))->Connect(ip, port);
}

inline void Disconnect(NETID net_id)
{
	std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"))->Disconnect(net_id);
}

inline bool Send(NETID net_id, const char * data, uint16_t size)
{
	return std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"))->Send(net_id, data, size);
}

}

NAMESPACE_CLOSE

#endif // USRV_SERVER_INTERFACE_HPP
