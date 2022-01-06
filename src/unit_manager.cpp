// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "unit_manager.h"

#include "unit.h"
#include "util/time.h"

NAMESPACE_OPEN

bool UnitManager::Register(const std::string & name, std::shared_ptr<Unit> && unit)
{
	if (_units.find(name) != _units.end())
	{
		return false;
	}

	unit->OnRegister(shared_from_this());

	_units.insert(std::make_pair(name, std::move(unit)));

	return true;
}

std::shared_ptr<Unit> UnitManager::Get(const std::string & name)
{
	auto iter = _units.find(name);
	if (iter == _units.end())
	{
		return nullptr;
	}
	return iter->second;
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
		if (!unit.second->Start())
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
		unit.second->Update(interval);
	}
}

void UnitManager::_Stop()
{
	for (auto & unit : _units)
	{
		unit.second->Stop();
	}
}

void UnitManager::_MainLoop()
{
	auto start = clock();
	auto now = start;
	auto interval = ClockMs(now - start);
	while (!_exit)
	{
		now = clock();
		interval = ClockMs(now - start);
		if (interval >= _interval)
		{
			_Update(interval);
			start = now;
		}
		else
		{
			usleep((_interval - interval) * (CLOCKS_PER_SEC / SEC2MILLISEC));
		}
	}
}

NAMESPACE_CLOSE
