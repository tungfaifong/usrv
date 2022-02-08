// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "unit_manager.h"

#include <chrono>
#include <string>

#include "interfaces/logger_interface.h"
#include "unit.h"
#include "util/time.h"

NAMESPACE_OPEN

void UnitManager::Init(intvl_t interval)
{
	_loop.Init(interval, [self = shared_from_this()](intvl_t interval){
		return self->_Update(interval);
	});
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
	if (!_Init())
	{
		LOGGER_FLUSH();
		return;
	}

	if (!_Start())
	{
		LOGGER_FLUSH();
		return;
	}

	_loop.Run();

	_Stop();

	_Release();

	_loop.Release();
}

void UnitManager::SetExit(bool exit)
{
	_loop.SetExit(exit);
}

intvl_t UnitManager::Interval()
{
	return _loop.Interval();
}

bool UnitManager::_Init()
{
	for(auto & [key, unit] : _units)
	{
		if(!unit->Init())
		{
			LOGGER_ERROR("UnitManager::Init {} fail", key);
			return false;
		}
		LOGGER_INFO("UnitManager::Init {} success", key);
	}
	LOGGER_INFO("UnitManager::Init All units success");
	return true;
}

bool UnitManager::_Start()
{
	for (auto & [key, unit] : _units)
	{
		if (!unit->Start())
		{
			LOGGER_ERROR("UnitManager::_Start {} fail", key);
			return false;
		}
		LOGGER_INFO("UnitManager::_Start {} success", key);
	}
	LOGGER_INFO("UnitManager::_Start All units success");
	return true;
}

bool UnitManager::_Update(intvl_t interval)
{
	auto busy = false;
	for (auto & [key, unit] : _units)
	{
		busy |= unit->Update(interval);
	}
	return busy;
}

void UnitManager::_Stop()
{
	for (auto & [key, unit] : _units)
	{
		unit->Stop();
		LOGGER_INFO("UnitManager::_Stop {} success", key);
	}
	LOGGER_INFO("UnitManager::_Stop All units success");
}

void UnitManager::_Release()
{
	for(auto & [key, unit] : _units)
	{
		unit->Release();
	}
}

NAMESPACE_CLOSE
