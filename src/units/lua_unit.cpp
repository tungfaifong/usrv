// Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

#include "lua_unit.h"

#include "interfaces/logger_interface.h"

NAMESPACE_OPEN

LuaUnit::LuaUnit(const std::string & file, LuaBindFunc bind_func) : _file(file), _lua_state(luaL_newstate()),
	_start(nullptr), _update(nullptr), _stop(nullptr), _on_recv(nullptr), _bind_func(bind_func)
{
	
}

bool LuaUnit::Init()
{
	luaL_openlibs(_lua_state);

	if (luaL_dofile(_lua_state, _file.c_str()))
	{
		logger::error("LuaUnit::Init error: missing file {}", _file);
		return false;
	}

	if (!_InitFunc(_start, "Start") || !_InitFunc(_update, "Update") || !_InitFunc(_stop, "Stop") || !_InitFunc(_on_recv, "OnRecv"))
	{
		logger::error("LuaUnit::Init error: _InitFunc filed.");
		return false;
	}

	if (_bind_func)
	{
		_bind_func(GetGlobalNamespace());
	}

	return true;
}

bool LuaUnit::Start()
{
	return _start();
}

void LuaUnit::Update(intvl_t interval)
{
	_update(interval);
}

void LuaUnit::Stop()
{
	_stop();
}

void LuaUnit::Release()
{
	lua_close(_lua_state);
	Unit::Release();
}

luabridge::LuaRef LuaUnit::GetGlobal(const char * name)
{
	return luabridge::getGlobal(_lua_state, name);
}

luabridge::Namespace LuaUnit::GetGlobalNamespace()
{
	return luabridge::getGlobalNamespace(_lua_state);
}

void LuaUnit::OnRecvFunc(NETID net_id, char * data, uint16_t size)
{
	_on_recv(net_id, data, size);
}

bool LuaUnit::_InitFunc(luabridge::LuaRef & func, const char * func_name)
{
	func = GetGlobal(func_name);
	if (!func.isFunction())
	{
		logger::error("lua func error: missing {}", func_name);
		return false;
	}
	return true;
}

NAMESPACE_CLOSE
