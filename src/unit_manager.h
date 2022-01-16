// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_UNIT_MANAGER_H
#define USRV_UNIT_MANAGER_H

#include <map>
#include <memory>

#include "components/loop.hpp"
#include "util/singleton.hpp"

NAMESPACE_OPEN

class Unit;

class UnitManager : public Singleton<UnitManager>, public std::enable_shared_from_this<UnitManager>
{
public:
	UnitManager() = default;
	~UnitManager() = default;

public:
	void Init(intvl_t interval);
	void Release();
	
	bool Register(const char * key, std::shared_ptr<Unit> && unit);
	std::shared_ptr<Unit> Get(const char * key);
	void Run();
	void SetExit(bool exit);
	intvl_t Interval();

private:
	bool _Start();
	void _Update(intvl_t interval);
	void _Stop();

private:
	Loop _loop;
	std::map<const char *, std::shared_ptr<Unit>> _units;
};

NAMESPACE_CLOSE

#endif // USRV_UNIT_MANAGER_H
