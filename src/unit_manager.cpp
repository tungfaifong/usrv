// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "unit_manager.h"

#include <chrono>
#include <string>

#include "interfaces/logger_interface.h"
#include "unit.h"
#include "util/time.h"

NAMESPACE_OPEN

void UnitManager::Init(size_t unit_num, const std::string * unit_keys)
{
	_units.resize(unit_num);
	_unit_keys = unit_keys;
}

bool UnitManager::Register(size_t key, std::shared_ptr<Unit> && unit)
{
	if(key >= _units.size())
	{
		return false;
	}

	unit->OnRegister(shared_from_this());

	_units[key] = std::move(unit);

	return true;
}

std::shared_ptr<Unit> UnitManager::Get(size_t key)
{
	if(key >= _units.size())
	{
		return nullptr;
	}
	return _units[key];
}

void UnitManager::Run(intvl_t interval)
{
	_interval = interval;

	if (!_Start())
	{
		logger::error("UnitManager::Run start fail.");
		return;
	}

	logger::info("UnitManager::Run start success.");

	_MainLoop();

	_Stop();
}

void UnitManager::SetExit(bool exit)
{
	_exit = exit;
}

intvl_t UnitManager::Interval()
{
	return _interval;
}

bool UnitManager::_Start()
{
	for (size_t i = 0; i < _units.size(); ++i)
	{
		if (!_units[i]->Start())
		{
			logger::error(fmt::format("UnitManager::_Start {} fail", _unit_keys[i]));
			return false;
		}
		logger::info(fmt::format("UnitManager::_Start {} success", _unit_keys[i]));
	}
	return true;
}

void UnitManager::_Update(intvl_t interval)
{
	for (auto & unit : _units)
	{
		unit->Update(interval);
	}
}

void UnitManager::_Stop()
{
	for (auto & unit : _units)
	{
		unit->Stop();
	}
}

void UnitManager::_MainLoop()
{
	auto start = SysNow();
	auto now = start;
	auto interval = Ns2Ms(now - start);
	while (!_exit)
	{
		now = SysNow();
		interval = Ns2Ms(now - start);
		if (interval >= _interval)
		{
			start = now;
			_Update(interval);
		}
		else
		{
			usleep((_interval - interval) * (CLOCKS_PER_SEC / SEC2MILLISEC));
		}
	}
}

NAMESPACE_CLOSE
