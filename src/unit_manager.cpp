// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "unit_manager.h"

#include <chrono>
#include <string>

#include "interfaces/logger_interface.h"
#include "util/time.h"
#include "unit.h"

NAMESPACE_OPEN

UnitManager::UnitManager(): _work_guard(asio::make_work_guard(_io_context)),
	_loop(_io_context), _update_time(NowMs())
{

}

void UnitManager::Init(intvl_t interval)
{
	_interval = interval;
	_loop.Init(_interval, [self = shared_from_this()](){
		return self->_Update();
	}, true);
}

bool UnitManager::Register(const std::string & key, std::shared_ptr<Unit> && unit)
{
	if(_units.find(key) != _units.end())
	{
		return false;
	}

	unit->OnRegister(shared_from_this());

	_units[key] = std::move(unit);

	return true;
}

std::shared_ptr<Unit> UnitManager::Get(const std::string & key)
{
	if(_units.find(key) == _units.end())
	{
		return nullptr;
	}
	return _units[key];
}

bool UnitManager::Run()
{
	auto ret = true;

	ret = _Init() && _Start();
	if(ret)
	{
		_io_context.run();
	}

	_Stop();

	_Release();

	return ret;
}

void UnitManager::SetExit(bool exit)
{
	if(exit)
	{
		_io_context.stop();
	}
}

intvl_t UnitManager::Interval()
{
	return _interval;
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

bool UnitManager::_Update()
{
	auto now = NowMs();
	intvl_t interval = now - _update_time;
	bool busy = false;
	for (auto & [key, unit] : _units)
	{
		busy |= unit->Update(interval);
	}
	_update_time = now;

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
	_units.clear();
}

NAMESPACE_CLOSE
