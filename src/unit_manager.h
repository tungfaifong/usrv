// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_UNIT_MANAGER_H
#define USRV_UNIT_MANAGER_H

#include <map>
#include <memory>

#include "util/common.h"

namespace usrv
{

class Unit;

class UnitManager
{
public:
	UnitManager(const UnitManager&) = delete;
	UnitManager& operator=(const UnitManager&) = delete;
	UnitManager(UnitManager&&) = delete;
	UnitManager& operator=(UnitManager&&) = delete;

private:
	UnitManager() = default;
	~UnitManager() = default;

public:
	static UnitManager * Instance();
	bool Register(const std::string & name, std::shared_ptr<Unit> && unit);
	std::shared_ptr<Unit> Get(const std::string &name);
	void Run(intvl_t interval);
	void SetExit(bool exit);

private:
	bool _Start();
	void _Update(intvl_t interval);
	void _Stop();
	void _MainLoop();

private:
	bool _exit = false;
	intvl_t _interval;
	std::map<std::string, std::shared_ptr<Unit>> _units;
};

}

#endif // USRV_UNIT_MANAGER_H
