// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "unit_manager.h"

#include <chrono>

#include "unit.h"
#include "util/time.h"

NAMESPACE_OPEN

UNITKEY UnitManager::Register(std::shared_ptr<Unit> && unit)
{
	auto key = _units.size();

	unit->OnRegister(shared_from_this());

	_units.emplace_back(std::move(unit));

	return key;
}

std::shared_ptr<Unit> UnitManager::Get(UNITKEY key)
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
		return;
	}

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
	for (auto & unit : _units)
	{
		if (!unit->Start())
		{
			return false;
		}
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
