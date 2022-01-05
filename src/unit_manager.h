// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_UNIT_MANAGER_H
#define USRV_UNIT_MANAGER_H

#include <map>
#include <memory>

#include "util/singleton.hpp"

NAMESPACE_OPEN

class Unit;

class UnitManager : public Singleton<UnitManager>, public std::enable_shared_from_this<UnitManager>
{
public:
	UnitManager() = default;
	~UnitManager() = default;

public:
	bool Register(const std::string & name, std::shared_ptr<Unit> && unit);
	std::shared_ptr<Unit> Get(const std::string & name);
	void Run(intvl_t interval);
	void SetExit(bool exit);
	intvl_t Interval();

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

NAMESPACE_CLOSE

#endif // USRV_UNIT_MANAGER_H
