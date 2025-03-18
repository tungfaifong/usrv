// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "unit_manager.h"

#include <chrono>
#include <string>

#include "interfaces/logger_interface.h"
#include "util/time.h"
#include "unit.h"

NAMESPACE_OPEN

UnitManager::UnitManager(): _work_guard(asio::make_work_guard(_io_context)),
	_update_time(NowMs()), _update_timer(_io_context)
{

}

void UnitManager::Init(intvl_t interval)
{
	_interval = interval;
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
		asio::co_spawn(_io_context, _Update(), asio::detached);
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

asio::awaitable<void> UnitManager::_Update()
{
	while(true)
	{
		auto now = NowMs();
		for (auto & [key, unit] : _units)
		{
			unit->Update(now - _update_time);
		}
		_update_time = now;

		_update_timer.expires_after(ms_t(_interval));
		asio::error_code ec;
		co_await _update_timer.async_wait(redirect_error(asio::use_awaitable, ec));
	}
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
