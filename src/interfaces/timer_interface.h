// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_TIMER_INTERFACE_H
#define USRV_TIMER_INTERFACE_H

#include "unit_manager.h"
#include "units/timer_unit.h"
#include "util/common.h"

NAMESPACE_OPEN

namespace timer
{

inline TIMERID CreateTimer(intvl_t time, std::function<void()> && callback)
{
	return std::dynamic_pointer_cast<TimerUnit>(UnitManager::Instance()->Get("TIMER"))->CreateTimer(time, std::move(callback));
}

inline bool CallTimer(TIMERID id)
{
	return std::dynamic_pointer_cast<TimerUnit>(UnitManager::Instance()->Get("TIMER"))->CallTimer(id);
}

inline bool RemoveTimer(TIMERID id)
{
	return std::dynamic_pointer_cast<TimerUnit>(UnitManager::Instance()->Get("TIMER"))->RemoveTimer(id);
}

}

NAMESPACE_CLOSE

#endif // USRV_TIMER_INTERFACE_H
