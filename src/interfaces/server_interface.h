// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_SERVER_INTERFACE_H
#define USRV_SERVER_INTERFACE_H

#include "units/server_unit.h"
#include "util/common.h"
#include "unit_manager.h"

NAMESPACE_OPEN

namespace server
{

inline void Listen(PORT port)
{
	std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"))->Listen(port);
}

inline void Connect(const IP & ip, PORT port, OnConnFunc callbcak)
{
	std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"))->Connect(ip, port, callbcak);
}

inline void Disconnect(NETID net_id)
{
	std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"))->Disconnect(net_id);
}

inline bool Send(NETID net_id, std::string msg)
{
	return std::dynamic_pointer_cast<ServerUnit>(UnitManager::Instance()->Get("SERVER"))->Send(net_id, std::move(msg));
}

}

NAMESPACE_CLOSE

#endif // USRV_SERVER_INTERFACE_H
