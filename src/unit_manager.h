// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#ifndef USRV_UNIT_MANAGER_H
#define USRV_UNIT_MANAGER_H

#include <map>
#include <memory>

#include "asio.hpp"

#include "util/singleton.hpp"

NAMESPACE_OPEN

class Unit;

class UnitManager : public Singleton<UnitManager>, public std::enable_shared_from_this<UnitManager>
{
public:
	UnitManager();
	~UnitManager() = default;

public:
	void Init(intvl_t interval);
	bool Register(const std::string & key, std::shared_ptr<Unit> && unit);
	std::shared_ptr<Unit> Get(const std::string & key);
	bool Run();
	void SetExit(bool exit);
	intvl_t Interval();
	asio::io_context & IOContext() { return _io_context; }

private:
	bool _Init();
	bool _Start();
	asio::awaitable<void> _Update();
	void _Stop();
	void _Release();

private:
	intvl_t _interval = 0;
	asio::io_context _io_context;
	asio::executor_work_guard<asio::io_context::executor_type> _work_guard;
	uint64_t _update_time;
	asio::steady_timer _update_timer;

	std::unordered_map<std::string, std::shared_ptr<Unit>> _units;
};

NAMESPACE_CLOSE

#endif // USRV_UNIT_MANAGER_H
