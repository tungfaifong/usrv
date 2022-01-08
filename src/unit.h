// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_UNIT_H
#define USRV_UNIT_H

#include <memory>

#include "util/common.h"

NAMESPACE_OPEN

class UnitManager;

class Unit
{
public:
	Unit() = default;
	virtual ~Unit() = default;

	virtual bool Start() = 0;
	virtual void Update(intvl_t interval) = 0;
	virtual void Stop() = 0;
	virtual void OnRegister(const std::shared_ptr<UnitManager> & mgr) { _mgr = mgr; }

protected:
	std::shared_ptr<UnitManager> _mgr;
};

NAMESPACE_CLOSE

#endif // USRV_UNIT_H
