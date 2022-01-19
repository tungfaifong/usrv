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

	virtual void OnRegister(const std::shared_ptr<UnitManager> & mgr) { _mgr = mgr; }
	virtual bool Init() = 0;
	virtual bool Start() = 0;
	virtual void Update(intvl_t interval) = 0;
	virtual void Stop() = 0;
	virtual void Release() = 0;

protected:
	std::shared_ptr<UnitManager> _mgr;
};

NAMESPACE_CLOSE

#endif // USRV_UNIT_H
