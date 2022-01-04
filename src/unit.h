// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_UNIT_H
#define USRV_UNIT_H

#include "util/common.h"

namespace usrv
{

class Unit
{
public:
	Unit() {}
	virtual ~Unit() {}

	virtual bool Start() = 0;
	virtual void Update(intvl_t interval) = 0;
	virtual void Stop() = 0;
};

}

#endif // USRV_UNIT_H
