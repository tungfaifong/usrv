// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "unit_manager.h"

#include <chrono>
#include <string>

#include "interfaces/logger_interface.hpp"
#include "unit.h"
#include "util/time.h"

NAMESPACE_OPEN

void UnitManager::Init(intvl_t interval)
{
	_loop.Init(interval, [self = shared_from_this()](intvl_t interval){
		self->_Update(interval);
	});

	for(auto & unit : _units)
	{
		unit.second->Init();
	}
}

void UnitManager::Release()
{
	for(auto & unit : _units)
	{
		unit.second->Release();
	}
}

bool UnitManager::Register(const char * key, std::shared_ptr<Unit> && unit)
{
	if(_units.find(key) != _units.end())
	{
		return false;
	}

	unit->OnRegister(shared_from_this());

	_units[key] = std::move(unit);

	return true;
}

std::shared_ptr<Unit> UnitManager::Get(const char * key)
{
	if(_units.find(key) == _units.end())
	{
		return nullptr;
	}
	return _units[key];
}

void UnitManager::Run()
{
	if (!_Start())
	{
		return;
	}

	_loop.Run();

	_Stop();
}

void UnitManager::SetExit(bool exit)
{
	_loop.SetExit(exit);
}

intvl_t UnitManager::Interval()
{
	return _loop.Interval();
}

bool UnitManager::_Start()
{
	for (auto & unit : _units)
	{
		if (!unit.second->Start())
		{
			logger::error("UnitManager::_Start {} fail", unit.first);
			return false;
		}
		logger::info("UnitManager::_Start {} success", unit.first);
	}
	logger::info("UnitManager::_Start All units success");
	return true;
}

void UnitManager::_Update(intvl_t interval)
{
	for (auto & unit : _units)
	{
		unit.second->Update(interval);
	}
}

void UnitManager::_Stop()
{
	for (auto & unit : _units)
	{
		unit.second->Stop();
		logger::info("UnitManager::_Stop {} success", unit.first);
	}
	logger::info("UnitManager::_Stop All units success");
}

NAMESPACE_CLOSE
